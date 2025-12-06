// unit_tests.cpp
// g++ -std=c++17 unit_tests.cpp src/modules/*.cpp -I src/include && ./a.out
#include <iostream>
#include <vector>
#include <string>

#include "src/include/modules/timestampconv.hpp"
#include "src/include/modules/tokens.hpp"
#include "src/include/modules/correctlineoffset.hpp"

// #include "src/include/debug.hpp"

using namespace std;

/* ---------- helpers ---------- */
template <typename T, typename U>               // <-- add U
void PRINT(const std::string& title, const T& in, const U& out)
{
    std::cout << title << "  in: \"" << in << "\"  out: \"" << out << "\"\n";
}

/* ---------- divide_timestamp ---------- */
void TEST_divide_timestamp()
{
    cout << "\n===== divide_timestamp =====\n";
    auto run = [](const string& s){
        tsmap m = divide_timestamp(s);
        cout << s << "  ->  mm=" << m["mm"]
             << " ss=" << m["ss"] << " ms=" << m["ms"] << "\n";
    };
    run("00:00.10");
    run("12:34.56");
    run("banana");          // empty map expected
    run("55.56.12");        // invalid → empty map
    run("00:15.25");
    run("01:10.01");
    run("1:2.3");
}

/* ---------- ms_to_timestamp ---------- */
void TEST_ms_to_timestamp()
{
    cout << "\n===== ms_to_timestamp =====\n";
    auto run = [](long ms){
        string ts = ms_to_timestamp(ms);
        PRINT("ms_to_timestamp", ms, ts);
    };
    run(1);
    run(10);
    run(1000);
    run(65000);
    run(3600000);
    run(123456);
    run(-565);   // negative → clamped to 0 internally
    run(-89);
    run(65536);
}

/* ---------- timestamp_to_ms ---------- */
void TEST_timestamp_to_ms()
{
    cout << "\n===== timestamp_to_ms =====\n";
    auto run = [](const string& ts){
        long ms = timestamp_to_ms(ts);
        PRINT("timestamp_to_ms", ts, ms);
    };
    run("00:00.00");
    run("00:01.00");
    run("01:00.00");
    run("34:35.00");
    run("12:34.56");
    run("65:13.27");
    run("23:24.35");

    /* a few more inferred cases */
    run("00:00.01");   // 10 ms
    run("00:10.00");   // 10 000 ms
    run("10:00.00");   // 600 000 ms
    run("99:59.99");   // edge max
    run("invalid");    // should return 0
}

/* ---------- is_it_a_timestamp ---------- */
void TEST_is_it_a_timestamp()
{
    cout << "\n===== is_it_a_timestamp =====\n";
    auto run = [](const string& s){
        bool ok = is_it_a_timestamp(s);
        PRINT("is_it_a_timestamp", s, ok ? "true" : "false");
    };
    // valid
    run("00:00.00");
    run("12:34.56");
    run("9:59.99");
    // invalid
    run("12-34.56");
    run("12:34:56");
    run("abc");
    run("");
    run("00:00a00");
    run("56.65:23");
    run("55.56.12");
}

/* ---------- tokenize_lyric_line ---------- */
void TEST_tokenize_lyric_line()
{
    cout << "\n===== tokenize_lyric_line =====\n";
    auto run = [](const string& line){
        vector<string> v = tokenize_lyric_line(line);
        cout << "LINE: \"" << line << "\"\nTOKENS: ";
        for(auto& t: v) cout << "{" << t << "} ";
        cout << "\n\n";
    };
    run("This is just plain text");
    run("[00:10:05] This is a lyric line");
    run("[12:34 This is a malformed line but should come fine anyway");
    run("<The jo>b of this function is to split by spaces or [these ]signs");
}

/* ---------- serialize_lyric_tokens ---------- */
void TEST_serialize_lyric_tokens()
{
    cout << "\n===== serialize_lyric_tokens =====\n";
    auto run = [](const vector<string>& v){
        string line = serialize_lyric_tokens(v);
        cout << "VECTOR → LINE: \"" << line << "\"\n";
    };
    run({"This", "is", "just", "plain", "text"});
    run({"[", "00:10:05", "]", "This", "is", "a", "lyric", "line"});
    run({"[", "12:34", "This", "is", "a", "malformed", "line", "but", "should", "come", "fine", "anyway"});
    run({"<", "The", "jo", ">", "b", "of", "this", "function", "is", "to", "split", "by", "spaces", "or", "[", "these", "]", "signs"});
}

/* ---------- apply_offset_to_timestamp ---------- */
void TEST_apply_offset_to_timestamp()
{
    cout << "\n===== apply_offset_to_timestamp (+1250 ms) =====\n";
    auto run = [](const string& ts, long offset, bool inv=false){
        string out = apply_offset_to_timestamp(ts, offset, inv);
        /*
        LOG("");
        LOG("timestamp in ms: " + std::to_string(timestamp_to_ms(ts)));
        LOG("sum of timestamps:", std::to_string(timestamp_to_ms(ts)) + " + " + std::to_string(offset) + " = " + std::to_string(timestamp_to_ms(ts) + offset));
        */
        PRINT("apply_offset", ts, out);
    };
    const long off = 1250;
    run("00:02.00", off);
    run("00:05.00", off);
    run("01:00.00", off);
    run("12:34.56", off);
    // extras
    run("00:00.00", off);
    run("00:59.99", off);
    run("99:59.99", off);
}

/* ---------- correct_line_offset ---------- */
void TEST_correct_line_offset()
{
    cout << "\n===== correct_line_offset (+750 ms, invert=true → add) =====\n";
    auto run = [](const string& line, long off, bool inv=true){
        string out = correct_line_offset(line, off, inv);
        PRINT("correct_line_offset", line, out);
    };
    const long off = 750;
    run("[00:00.00] First line", off);
    run("[65:05.36] Test line", off);
    run("[048:34:35] Goofed timestamp that is going to be left as is", off);
    run("[3252:3405:405] Another untouched timestamp", off);
}

/* ---------- main driver ---------- */
int main()
{
    TEST_is_it_a_timestamp();
    TEST_divide_timestamp();
    TEST_ms_to_timestamp();
    TEST_timestamp_to_ms();
    TEST_tokenize_lyric_line();
    TEST_serialize_lyric_tokens();
    TEST_apply_offset_to_timestamp();
    TEST_correct_line_offset();
    return 0;
}