void testArrayParser()
{
    // ============================================================
    // 1. EMPTY ARRAY
    // ============================================================

    {
        char buffer[] = "*0\r\n";

        parseResult res = arrayParser(buffer, sizeof(buffer) - 1);

        assert(res.parserRes == COMPLETED);
        assert(res.respValue.dataType == ::array);
        assert(get<vector<respValueStruct>>(
                   res.respValue.respValue).empty());
        assert(res.bytesConsumed == 4);
    }


    // ============================================================
    // 2. NULL ARRAY
    // ============================================================

    {
        char buffer[] = "*-1\r\n";

        parseResult res = arrayParser(buffer, sizeof(buffer) - 1);

        assert(res.parserRes == COMPLETED);
        assert(res.respValue.dataType == ::array);
        assert(holds_alternative<monostate>(
                   res.respValue.respValue));
        assert(res.bytesConsumed == 5);
    }


    // ============================================================
    // 3. ARRAY WITH ONE INTEGER
    // ============================================================

    {
        char buffer[] = "*1\r\n:42\r\n";

        parseResult res = arrayParser(buffer, sizeof(buffer) - 1);

        assert(res.parserRes == COMPLETED);
        assert(res.respValue.dataType == ::array);

        auto values =
            get<vector<respValueStruct>>(res.respValue.respValue);

        assert(values.size() == 1);
        assert(values[0].dataType == integers);
        assert(get<int64_t>(values[0].respValue) == 42);

        assert(res.bytesConsumed == 9);
    }


    // ============================================================
    // 4. ARRAY WITH MULTIPLE INTEGERS
    // ============================================================

    {
        char buffer[] =
            "*3\r\n"
            ":10\r\n"
            ":20\r\n"
            ":30\r\n";

        parseResult res = arrayParser(buffer, sizeof(buffer) - 1);

        assert(res.parserRes == COMPLETED);

        auto values =
            get<vector<respValueStruct>>(res.respValue.respValue);

        assert(values.size() == 3);

        assert(get<int64_t>(values[0].respValue) == 10);
        assert(get<int64_t>(values[1].respValue) == 20);
        assert(get<int64_t>(values[2].respValue) == 30);
    }


    // ============================================================
    // 5. MIXED TYPES
    // ============================================================

    {
        char buffer[] =
            "*4\r\n"
            ":42\r\n"
            "+OK\r\n"
            "$3\r\nfoo\r\n"
            "-ERR bad\r\n";

        parseResult res = arrayParser(buffer, sizeof(buffer) - 1);

        assert(res.parserRes == COMPLETED);

        auto values =
            get<vector<respValueStruct>>(res.respValue.respValue);

        assert(values.size() == 4);

        assert(values[0].dataType == integers);
        assert(get<int64_t>(values[0].respValue) == 42);

        assert(values[1].dataType == simpleString);
        assert(get<string>(values[1].respValue) == "OK");

        assert(values[2].dataType == bulkString);
        assert(get<string>(values[2].respValue) == "foo");

        assert(values[3].dataType == error);
        assert(get<string>(values[3].respValue) == "ERR bad");
    }


    // ============================================================
    // 6. EMPTY BULK STRING INSIDE ARRAY
    // ============================================================

    {
        char buffer[] =
            "*1\r\n"
            "$0\r\n\r\n";

        parseResult res = arrayParser(buffer, sizeof(buffer) - 1);

        assert(res.parserRes == COMPLETED);

        auto values =
            get<vector<respValueStruct>>(res.respValue.respValue);

        assert(values.size() == 1);
        assert(values[0].dataType == bulkString);
        assert(get<string>(values[0].respValue) == "");
    }


    // ============================================================
    // 7. NULL BULK STRING INSIDE ARRAY
    // ============================================================

    {
        char buffer[] =
            "*1\r\n"
            "$-1\r\n";

        parseResult res = arrayParser(buffer, sizeof(buffer) - 1);

        assert(res.parserRes == COMPLETED);

        auto values =
            get<vector<respValueStruct>>(res.respValue.respValue);

        assert(values.size() == 1);
        assert(values[0].dataType == bulkString);

        assert(holds_alternative<monostate>(
                   values[0].respValue));
    }


    // ============================================================
    // 8. NESTED ARRAY
    // ============================================================

    {
        char buffer[] =
            "*2\r\n"
            "*2\r\n"
            ":1\r\n"
            ":2\r\n"
            "*1\r\n"
            "+OK\r\n";

        parseResult res = arrayParser(buffer, sizeof(buffer) - 1);

        assert(res.parserRes == COMPLETED);

        auto outer =
            get<vector<respValueStruct>>(res.respValue.respValue);

        assert(outer.size() == 2);

        // First element is an array
        assert(outer[0].dataType == ::array);

        auto inner =
            get<vector<respValueStruct>>(outer[0].respValue);

        assert(inner.size() == 2);
        assert(get<int64_t>(inner[0].respValue) == 1);
        assert(get<int64_t>(inner[1].respValue) == 2);

        // Second element is another array
        assert(outer[1].dataType == ::array);

        auto inner2 =
            get<vector<respValueStruct>>(outer[1].respValue);

        assert(inner2.size() == 1);
        assert(get<string>(inner2[0].respValue) == "OK");
    }


    // ============================================================
    // 9. INCOMPLETE ARRAY HEADER
    // ============================================================

    {
        char buffer[] = "*";

        parseResult res = arrayParser(buffer, sizeof(buffer) - 1);

        assert(res.parserRes == INCOMPLETE);
        assert(res.bytesConsumed == 1);
    }


    // ============================================================
    // 10. INCOMPLETE ARRAY LENGTH
    // ============================================================

    {
        char buffer[] = "*2";

        parseResult res = arrayParser(buffer, sizeof(buffer) - 1);

        assert(res.parserRes == INCOMPLETE);
        assert(res.bytesConsumed == 2);
    }


    // ============================================================
    // 11. ARRAY HEADER ENDS WITH '\r'
    // ============================================================

    {
        char buffer[] = "*2\r";

        parseResult res = arrayParser(buffer, sizeof(buffer) - 1);

        assert(res.parserRes == INCOMPLETE);
        assert(res.bytesConsumed == 3);
    }


    // ============================================================
    // 12. INCOMPLETE FIRST ELEMENT
    // ============================================================

    {
        char buffer[] =
            "*1\r\n"
            ":42";

        parseResult res = arrayParser(buffer, sizeof(buffer) - 1);

        assert(res.parserRes == INCOMPLETE);
    }


    // ============================================================
    // 13. INCOMPLETE SECOND ELEMENT
    // ============================================================

    {
        char buffer[] =
            "*2\r\n"
            ":42\r\n"
            "$5\r\nhel";

        parseResult res = arrayParser(buffer, sizeof(buffer) - 1);

        assert(res.parserRes == INCOMPLETE);
    }


    // ============================================================
    // 14. MALFORMED ARRAY TYPE
    // ============================================================

    {
        char buffer[] = "+2\r\n";

        parseResult res = arrayParser(buffer, sizeof(buffer) - 1);

        assert(res.parserRes == MALFORMED);
        assert(res.bytesConsumed == 1);
    }


    // ============================================================
    // 15. MALFORMED ARRAY LENGTH
    // ============================================================

    {
        char buffer[] = "*abc\r\n";

        parseResult res = arrayParser(buffer, sizeof(buffer) - 1);

        assert(res.parserRes == MALFORMED);
    }


    // ============================================================
    // 16. MALFORMED ARRAY LENGTH: NO NUMBER
    // ============================================================

    {
        char buffer[] = "*\r\n";

        parseResult res = arrayParser(buffer, sizeof(buffer) - 1);

        assert(res.parserRes == MALFORMED);
    }


    // ============================================================
    // 17. MALFORMED CRLF
    // ============================================================

    {
        char buffer[] = "*2\rX";

        parseResult res = arrayParser(buffer, sizeof(buffer) - 1);

        assert(res.parserRes == MALFORMED);
    }


    // ============================================================
    // 18. MALFORMED ELEMENT TYPE
    // ============================================================

    {
        char buffer[] =
            "*1\r\n"
            "@something\r\n";

        parseResult res = arrayParser(buffer, sizeof(buffer) - 1);

        assert(res.parserRes == MALFORMED);
    }


    // ============================================================
    // 19. MALFORMED ELEMENT
    // ============================================================

    {
        char buffer[] =
            "*1\r\n"
            ":abc\r\n";

        parseResult res = arrayParser(buffer, sizeof(buffer) - 1);

        assert(res.parserRes == MALFORMED);
    }


    // ============================================================
    // 20. MULTIPLE RESP VALUES AFTER ARRAY
    //
    // This is VERY important for bytesConsumed.
    // ============================================================

    {
        char buffer[] =
            "*2\r\n"
            ":10\r\n"
            ":20\r\n"
            "+OK\r\n";

        int bufferSize = sizeof(buffer) - 1;

        parseResult res =
            arrayParser(buffer, bufferSize);

        assert(res.parserRes == COMPLETED);

        // Array should consume only itself,
        // NOT the following +OK\r\n.
        int expectedArrayBytes =
            4 + 5 + 5;   // *2\r\n + :10\r\n + :20\r\n

        assert(res.bytesConsumed == expectedArrayBytes);

        // Verify that the next RESP value starts exactly
        // where arrayParser stopped.
        int remainingBytes =
            bufferSize - res.bytesConsumed;

        parseResult next =
            simpleStringParser(
                buffer + res.bytesConsumed,
                remainingBytes
            );

        assert(next.parserRes == COMPLETED);
        assert(get<string>(next.respValue.respValue) == "OK");
        assert(next.bytesConsumed == 5);
    }


    // ============================================================
    // 21. MULTIPLE ARRAYS IN ONE BUFFER
    // ============================================================

    {
        char buffer[] =
            "*1\r\n:10\r\n"
            "*1\r\n:20\r\n";

        int bufferSize = sizeof(buffer) - 1;
        int totalBytesConsumed = 0;

        parseResult first =
            arrayParser(
                buffer + totalBytesConsumed,
                bufferSize - totalBytesConsumed
            );

        assert(first.parserRes == COMPLETED);

        totalBytesConsumed += first.bytesConsumed;

        parseResult second =
            arrayParser(
                buffer + totalBytesConsumed,
                bufferSize - totalBytesConsumed
            );

        assert(second.parserRes == COMPLETED);

        totalBytesConsumed += second.bytesConsumed;

        assert(totalBytesConsumed == bufferSize);
    }


    // ============================================================
    // 22. NULL ARRAY FOLLOWED BY ANOTHER VALUE
    // ============================================================

    {
        char buffer[] =
            "*-1\r\n"
            "+OK\r\n";

        int bufferSize = sizeof(buffer) - 1;

        parseResult res =
            arrayParser(buffer, bufferSize);

        assert(res.parserRes == COMPLETED);
        assert(res.bytesConsumed == 5);

        parseResult next =
            simpleStringParser(
                buffer + res.bytesConsumed,
                bufferSize - res.bytesConsumed
            );

        assert(next.parserRes == COMPLETED);
        assert(get<string>(next.respValue.respValue) == "OK");
    }


    // ============================================================
    // 23. DEEPLY NESTED ARRAY
    // ============================================================

    {
        char buffer[] =
            "*1\r\n"
            "*1\r\n"
            "*1\r\n"
            ":42\r\n";

        parseResult res = arrayParser(buffer, sizeof(buffer) - 1);

        assert(res.parserRes == COMPLETED);

        auto level1 =
            get<vector<respValueStruct>>(res.respValue.respValue);

        assert(level1.size() == 1);

        auto level2 =
            get<vector<respValueStruct>>(level1[0].respValue);

        assert(level2.size() == 1);

        auto level3 =
            get<vector<respValueStruct>>(level2[0].respValue);

        assert(level3.size() == 1);

        assert(get<int64_t>(level3[0].respValue) == 42);
    }


    // ============================================================
    // 24. ARRAY WITH ALL RESP TYPES
    // ============================================================

    {
        char buffer[] =
            "*5\r\n"
            ":123\r\n"
            "+OK\r\n"
            "$3\r\nfoo\r\n"
            "-ERR bad\r\n"
            "*0\r\n";

        parseResult res = arrayParser(buffer, sizeof(buffer) - 1);

        assert(res.parserRes == COMPLETED);

        auto values =
            get<vector<respValueStruct>>(res.respValue.respValue);

        assert(values.size() == 5);

        assert(values[0].dataType == integers);
        assert(values[1].dataType == simpleString);
        assert(values[2].dataType == bulkString);
        assert(values[3].dataType == error);
        assert(values[4].dataType == ::array);
    }


    cout << "All array parser tests passed!" << endl;
}
