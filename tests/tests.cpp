// unit_tests.cpp
// g++ -std=c++17 unit_tests.cpp src/modules/*.cpp -I src/include && ./a.out
#include <iostream>
#include <vector>
#include <string>

#include "modules/lrcfilerewriter.hpp"
#include "modules/timestampconv.hpp"
#include "modules/tokens.hpp"
#include "modules/correctlineoffset.hpp"

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
    run("250");
    run("1000");
}

/* ---------- tokenize_lyric_line ---------- */
void TEST_tokenize_lyric_line()
{
    cout << "\n===== tokenize_lyric_line =====\n";
    auto run = [](const string& line){
        vector<string> v = tokenize_line(line, true);
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
        string line = serialize_tokens(v, " ", true);
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

/* ---------- read_tags_from_line ---------- */
void TEST_read_tags_from_line()
{
    cout << "\n===== read_tags_from_line =====\n";
    auto run = [](const string& line){
        vector<pair<string,string>> tags = read_tags_from_line(line);
        cout << "LINE: \"" << line << "\"\nTAGS: ";
        for (auto& [k,v] : tags) cout << "'" << k << ": " << v << "' - ";
        cout << "\n\n";
    };

    /* basic id3-like tags */
    run("[ti: Song name] lyrics");
    run("[ar: Artist][al: Album][offset: 750]");
    run("[offset:-2315]");

    /* timestamp lines (should yield empty tag name) */
    run("[01:53.00] Si de mí todo entregué");
    run("[00:00.00] Start");

    /* edge cases */
    run("[al:$AD BOYZ 4 LIFE II]");
    run("[]");                       // empty brackets
    run("no brackets at all");       // plain text
    run("[malformed");               // missing closing
    run("[re:Replay:Extra]");        // colon inside value
}

/* ---------- helpers ---------- */
static filelines split_multiline(const std::string& src)
{
    filelines v;
    std::stringstream ss(src);
    std::string line;
    while (std::getline(ss, line)) v.push_back(line);
    return v;
}

/* ---------- process_lyrics (vector) ---------- */
void TEST_process_lyrics_vector()
{
    cout << "\n===== process_lyrics (vector overload) =====\n";

    auto run = [&](const string& block, const string& opts){
        filelines in  = split_multiline(block);
        filelines out = process_lyrics(in, opts);
        cout << "INPUT:\n" << block << "\nOPTIONS: \"" << opts << "\"\nOUTPUT:\n";
        for (auto& l : out) cout << l << '\n';
        cout << string(40, '-') << '\n';
    };

    /* INPUT 1 ----------------------------------------------------------------- */
    run(R"([offset: 750]
[00:40.10]She was cryin’ on my shoulder
[00:43.20]All I could do was hold her
[00:46.50]Only made us closer until July
[00:53.40]Now I know that you love me
[00:56.20]You don't need to remind me
[00:59.40]I should put it all behind me, shouldn't I?)", "correctoffset");

    /* INPUT 2 (add invertoffset) --------------------------------------------- */
    run(R"([offset: -1500]
[02:37.28]Two thousand years and twenty more
[02:40.05]And I thought I was weaker than I ever was before
[02:45.28]But the moonlight's rays hit the water below
[02:49.54]And it was now or never to swim to the Garden, and oh
[02:53.94]Did she push and pull, tryin' to get me to drown?
[02:58.49]But the lilies in my mind began to run aground
[03:02.69]As I gripped and dragged myself along the dirt
[03:06.95]I turned back to face the girl I loved for all of my hurt
[03:11.24]Said, "My dear, this is it, I have got to go"
[03:15.71]And she faded in the deep and murky water below)", "correctoffset invertoffset");

    /* INPUT 3 ----------------------------------------------------------------- */
    run(R"([offset: 250]
[03:04.20]And I was running far away
[03:06.40]Would I run off the world someday?
[03:09.00]But now take me home
[03:10.50]Take me home where I belong
[03:14.80]I got no other place to go
[03:17.10]Now take me home
[03:19.00]Take me home where I belong
[03:23.20]I got no other place to go)", "correctoffset");
}

/* ---------- process_lyrics (file) ---------- */
void TEST_process_lyrics_file()
{
    cout << "\n===== process_lyrics (file overload) =====\n";

    auto run = [&](const fs::path& p, const string& opts){
        cout << "FILE: " << p << "  OPTIONS: \"" << opts << "\"\nOUTPUT:\n";
        filelines out = process_lyrics(p, opts);
        for (auto& l : out) cout << l << '\n';
        cout << string(40, '-') << '\n';
    };

    run("test1.lrc", "correctoffset");
    run("test2.lrc", "correctoffset");
    run("test3.lrc", "correctoffset invertoffset");
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
    TEST_read_tags_from_line();
    TEST_process_lyrics_vector();
    TEST_process_lyrics_file();
    return 0;
}