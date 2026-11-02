// Json.h - tiny JSON value, parser and writer (no external library needed)
#pragma once
#include <string>
#include <vector>
#include <utility>
#include <cstdio>
#include <cstdlib>
#include <cmath>

struct Json {
    enum Type { Null, Bool, Num, Str, Arr, Obj } type = Null;
    bool b = false;
    double n = 0;
    std::string s;
    std::vector<Json> a;
    std::vector<std::pair<std::string, Json>> o;

    Json() {}
    Json(bool v) : type(Bool), b(v) {}
    Json(int v) : type(Num), n(v) {}
    Json(size_t v) : type(Num), n((double)v) {}
    Json(double v) : type(Num), n(v) {}
    Json(const char* v) : type(Str), s(v) {}
    Json(const std::string& v) : type(Str), s(v) {}

    static Json object() { Json j; j.type = Obj; return j; }
    static Json array() { Json j; j.type = Arr; return j; }

    Json& operator[](const std::string& key) {
        type = Obj;
        for (auto& p : o) if (p.first == key) return p.second;
        o.push_back({key, Json()});
        return o.back().second;
    }
    const Json* find(const std::string& key) const {
        if (type != Obj) return nullptr;
        for (auto& p : o) if (p.first == key) return &p.second;
        return nullptr;
    }
    void push(const Json& v) { type = Arr; a.push_back(v); }

    std::string getStr(const std::string& k, const std::string& d = "") const {
        const Json* j = find(k); return (j && j->type == Str) ? j->s : d;
    }
    int getInt(const std::string& k, int d = 0) const {
        const Json* j = find(k);
        if (j && j->type == Num) return (int)j->n;
        if (j && j->type == Str) return atoi(j->s.c_str());
        return d;
    }
    double getNum(const std::string& k, double d = 0) const {
        const Json* j = find(k); return (j && j->type == Num) ? j->n : d;
    }
    std::vector<std::string> getStrList(const std::string& k) const {
        std::vector<std::string> r;
        const Json* j = find(k);
        if (j && j->type == Arr) for (auto& x : j->a) if (x.type == Str) r.push_back(x.s);
        return r;
    }

    static void escape(const std::string& in, std::string& out) {
        out += '"';
        for (unsigned char c : in) {
            switch (c) {
                case '"': out += "\\\""; break;
                case '\\': out += "\\\\"; break;
                case '\n': out += "\\n"; break;
                case '\r': out += "\\r"; break;
                case '\t': out += "\\t"; break;
                default:
                    if (c < 0x20) { char buf[8]; snprintf(buf, sizeof buf, "\\u%04x", c); out += buf; }
                    else out += (char)c;
            }
        }
        out += '"';
    }
    void dump(std::string& out) const {
        switch (type) {
            case Null: out += "null"; break;
            case Bool: out += b ? "true" : "false"; break;
            case Num: {
                char buf[64];
                if (std::fabs(n - std::floor(n)) < 1e-9 && std::fabs(n) < 1e15) snprintf(buf, sizeof buf, "%lld", (long long)n);
                else snprintf(buf, sizeof buf, "%.2f", n);
                out += buf; break;
            }
            case Str: escape(s, out); break;
            case Arr:
                out += '[';
                for (size_t i = 0; i < a.size(); i++) { if (i) out += ','; a[i].dump(out); }
                out += ']'; break;
            case Obj:
                out += '{';
                for (size_t i = 0; i < o.size(); i++) {
                    if (i) out += ',';
                    escape(o[i].first, out); out += ':'; o[i].second.dump(out);
                }
                out += '}'; break;
        }
    }
    std::string dump() const { std::string r; dump(r); return r; }

    static Json parse(const std::string& text, bool* ok = nullptr) {
        size_t i = 0; bool good = true;
        Json j = parseValue(text, i, good);
        skipWs(text, i);
        if (i != text.size()) good = false;
        if (ok) *ok = good;
        return good ? j : Json();
    }
private:
    static void skipWs(const std::string& t, size_t& i) {
        while (i < t.size() && (t[i] == ' ' || t[i] == '\n' || t[i] == '\r' || t[i] == '\t')) i++;
    }
    static void putUtf8(std::string& out, unsigned cp) {
        if (cp < 0x80) out += (char)cp;
        else if (cp < 0x800) { out += (char)(0xC0 | (cp >> 6)); out += (char)(0x80 | (cp & 0x3F)); }
        else { out += (char)(0xE0 | (cp >> 12)); out += (char)(0x80 | ((cp >> 6) & 0x3F)); out += (char)(0x80 | (cp & 0x3F)); }
    }
    static std::string parseString(const std::string& t, size_t& i, bool& ok) {
        std::string r; i++;
        while (i < t.size() && t[i] != '"') {
            if (t[i] == '\\' && i + 1 < t.size()) {
                i++;
                switch (t[i]) {
                    case 'n': r += '\n'; break; case 't': r += '\t'; break; case 'r': r += '\r'; break;
                    case 'b': r += '\b'; break; case 'f': r += '\f'; break;
                    case 'u': if (i + 4 < t.size()) { putUtf8(r, (unsigned)strtoul(t.substr(i + 1, 4).c_str(), nullptr, 16)); i += 4; } break;
                    default: r += t[i];
                }
            } else r += t[i];
            i++;
        }
        if (i >= t.size()) ok = false; else i++;
        return r;
    }
    static Json parseValue(const std::string& t, size_t& i, bool& ok) {
        skipWs(t, i);
        if (i >= t.size()) { ok = false; return Json(); }
        char c = t[i];
        if (c == '{') {
            Json j = object(); i++; skipWs(t, i);
            if (i < t.size() && t[i] == '}') { i++; return j; }
            while (ok && i < t.size()) {
                skipWs(t, i);
                if (i >= t.size() || t[i] != '"') { ok = false; break; }
                std::string k = parseString(t, i, ok); skipWs(t, i);
                if (i >= t.size() || t[i] != ':') { ok = false; break; }
                i++; Json v = parseValue(t, i, ok);
                j.o.push_back({k, v}); skipWs(t, i);
                if (i < t.size() && t[i] == ',') { i++; continue; }
                if (i < t.size() && t[i] == '}') { i++; break; }
                ok = false;
            }
            return j;
        }
        if (c == '[') {
            Json j = array(); i++; skipWs(t, i);
            if (i < t.size() && t[i] == ']') { i++; return j; }
            while (ok && i < t.size()) {
                j.a.push_back(parseValue(t, i, ok)); skipWs(t, i);
                if (i < t.size() && t[i] == ',') { i++; continue; }
                if (i < t.size() && t[i] == ']') { i++; break; }
                ok = false;
            }
            return j;
        }
        if (c == '"') return Json(parseString(t, i, ok));
        if (t.compare(i, 4, "true") == 0) { i += 4; return Json(true); }
        if (t.compare(i, 5, "false") == 0) { i += 5; return Json(false); }
        if (t.compare(i, 4, "null") == 0) { i += 4; return Json(); }
        char* end; double d = strtod(t.c_str() + i, &end);
        if (end == t.c_str() + i) { ok = false; return Json(); }
        i = end - t.c_str(); return Json(d);
    }
};
