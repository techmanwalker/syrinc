#pragma once

#include <iostream>
#include <string>
#include <vector>

#ifdef DEBUG_BUILD

/* ---------- real implementations (Debug) ---------- */

/* ---------- scalars ---------- */
inline void LOG(long v,               const std::string& lvl = "info") {
    if(!lvl.empty()) std::cout << lvl << ": ";
    std::cout << v << '\n';
}
inline void LOG(int v,                const std::string& lvl = "info") {
    if(!lvl.empty()) std::cout << lvl << ": ";
    std::cout << v << '\n';
}
inline void LOG(double v,             const std::string& lvl = "info") {
    if(!lvl.empty()) std::cout << lvl << ": ";
    std::cout << v << '\n';
}
inline void LOG(const std::string& v, const std::string& lvl = "info") {
    if(v.empty()) { std::cout << '\n'; return; }
    if(!lvl.empty()) std::cout << lvl << ": ";
    std::cout << v << '\n';
}


/* ---------- vector<T> ---------- */
template <typename T>
void LOG(const std::vector<T>& v, const std::string& lvl = "info") {
    if(!lvl.empty()) std::cout << lvl << ": ";
    std::cout << '{';
    for(std::size_t i = 0; i < v.size(); ++i) {
        std::cout << v[i];
        if(i + 1 != v.size()) std::cout << ", ";
    }
    std::cout << "}\n";
}


/* ---------- variadic plain overloads (optional) ---------- */
//inline void LOG(const std::string& lvl) { if(!lvl.empty()) std::cout << lvl << ": "; std::cout << '\n'; }

#else 

/* ---------- no-op stubs (Release) ---------- */

using ___iostream___ = std::iostream;
inline void LOG(long,               const std::string& = "") {}
inline void LOG(int,                const std::string& = "") {}
inline void LOG(double,             const std::string& = "") {}
inline void LOG(const std::string&, const std::string& = "") {}
template <typename T>
void LOG(const std::vector<T>&, const std::string& = "") {}
#endif