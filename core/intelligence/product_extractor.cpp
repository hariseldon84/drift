#include "product_extractor.h"
#include "json.hpp"
#include <algorithm>
#include <cctype>
#include <chrono>
#include <ctime>
#include <regex>
#include <sstream>
#include <unordered_map>

using json = nlohmann::json;

namespace drift::intelligence {

// ── Helpers ───────────────────────────────────────────────────────────────────

static std::string iso8601_now() {
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    char buf[32] = {};
    std::strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%SZ", std::gmtime(&t));
    return buf;
}

static std::string domain_from_url(const std::string& url) {
    auto s = url.find("://");
    if (s == std::string::npos) return url;
    s += 3;
    auto e = url.find('/', s);
    std::string host = (e == std::string::npos) ? url.substr(s) : url.substr(s, e - s);
    if (host.size() > 4 && host.substr(0, 4) == "www.") host = host.substr(4);
    return host;
}

static std::string str_lower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return s;
}

static std::string json_str(const json& j, const std::string& key) {
    if (!j.contains(key)) return {};
    const auto& v = j[key];
    if (v.is_string()) return v.get<std::string>();
    if (v.is_number()) {
        std::ostringstream ss;
        ss << v.get<double>();
        return ss.str();
    }
    return {};
}

static double json_num(const json& j, const std::string& key) {
    if (!j.contains(key)) return 0.0;
    const auto& v = j[key];
    if (v.is_number()) return v.get<double>();
    if (v.is_string()) {
        try { return std::stod(v.get<std::string>()); } catch (...) {}
    }
    return 0.0;
}

// ── JSON-LD extraction ────────────────────────────────────────────────────────

static std::vector<std::string> find_json_ld_blocks(const std::string& html) {
    std::vector<std::string> out;
    std::string tag = "application/ld+json";
    std::size_t pos = 0;
    while ((pos = html.find(tag, pos)) != std::string::npos) {
        auto close_tag = html.find('>', pos);
        if (close_tag == std::string::npos) break;
        auto end_script = html.find("</script>", close_tag);
        if (end_script == std::string::npos) break;
        std::string content = html.substr(close_tag + 1, end_script - close_tag - 1);
        auto first = content.find('{');
        auto first_arr = content.find('[');
        std::size_t start = std::min(first, first_arr);
        if (start != std::string::npos) out.push_back(content.substr(start));
        pos = end_script + 9;
    }
    return out;
}

static void fill_offer(const json& offer, Product& p) {
    if (p.price_amount == 0.0) {
        auto price_s = json_str(offer, "price");
        if (!price_s.empty()) {
            std::string digits;
            for (char c : price_s)
                if (std::isdigit(static_cast<unsigned char>(c)) || c == '.') digits += c;
            try { p.price_amount = std::stod(digits); } catch (...) {}
        }
    }
    if (p.price_currency.empty() || p.price_currency == "USD") {
        auto cur = json_str(offer, "priceCurrency");
        if (!cur.empty()) p.price_currency = cur;
    }
    if (p.availability.empty()) {
        auto avail = json_str(offer, "availability");
        if (avail.find("InStock") != std::string::npos || avail == "in_stock")
            p.availability = "in_stock";
        else if (avail.find("OutOfStock") != std::string::npos || avail == "out_of_stock")
            p.availability = "out_of_stock";
        else if (avail.find("LimitedAvailability") != std::string::npos || avail == "limited")
            p.availability = "limited";
    }
}

static double fill_from_json_ld_obj(const json& obj, Product& p) {
    if (!obj.is_object()) return 0.0;

    bool is_product = false;
    if (obj.contains("@type")) {
        const auto& t = obj["@type"];
        if (t.is_string() && str_lower(t.get<std::string>()) == "product") is_product = true;
        if (t.is_array())
            for (const auto& el : t)
                if (el.is_string() && str_lower(el.get<std::string>()) == "product") is_product = true;
    }
    if (!is_product) return 0.0;

    p.name = json_str(obj, "name");

    if (obj.contains("brand")) {
        const auto& b = obj["brand"];
        if (b.is_string()) p.brand = b.get<std::string>();
        else if (b.is_object()) p.brand = json_str(b, "name");
    }

    if (obj.contains("offers")) {
        const auto& offers = obj["offers"];
        if (offers.is_object()) fill_offer(offers, p);
        else if (offers.is_array() && !offers.empty()) fill_offer(offers[0], p);
    }

    if (obj.contains("aggregateRating")) {
        const auto& ar = obj["aggregateRating"];
        if (ar.is_object()) {
            p.rating_score = json_num(ar, "ratingValue");
            auto cnt = json_num(ar, "reviewCount");
            if (cnt == 0.0) cnt = json_num(ar, "ratingCount");
            p.rating_count = static_cast<int>(cnt);
        }
    }

    if (obj.contains("image") && p.images.empty()) {
        const auto& img = obj["image"];
        if (img.is_string()) p.images.push_back(img.get<std::string>());
        else if (img.is_array()) {
            for (const auto& el : img) {
                if (p.images.size() >= 5) break;
                if (el.is_string()) p.images.push_back(el.get<std::string>());
                else if (el.is_object() && el.contains("url"))
                    p.images.push_back(json_str(el, "url"));
            }
        }
    }

    if (p.key_specs.empty() && obj.contains("description")) {
        std::istringstream ss(json_str(obj, "description"));
        std::string segment;
        while (std::getline(ss, segment, '.') && p.key_specs.size() < 5) {
            auto f = segment.find_first_not_of(" \t\r\n");
            auto l = segment.find_last_not_of(" \t\r\n");
            if (f != std::string::npos && (l - f) > 10)
                p.key_specs.push_back(segment.substr(f, l - f + 1));
        }
    }

    int filled = (!p.name.empty() ? 1 : 0) + (p.price_amount > 0.0 ? 1 : 0)
               + (!p.images.empty() ? 1 : 0) + (p.rating_score > 0.0 ? 1 : 0)
               + (!p.brand.empty() ? 1 : 0);
    return 0.5 + 0.1 * filled;  // 0.5–1.0
}

static double fill_from_json_ld(const std::string& html, Product& p) {
    for (const auto& block : find_json_ld_blocks(html)) {
        try {
            auto j = json::parse(block);
            auto try_obj = [&](const json& o) { return fill_from_json_ld_obj(o, p); };
            double conf = 0.0;
            if (j.is_object()) {
                conf = try_obj(j);
                if (conf == 0.0 && j.contains("@graph") && j["@graph"].is_array())
                    for (const auto& el : j["@graph"]) { conf = try_obj(el); if (conf > 0.0) break; }
            } else if (j.is_array()) {
                for (const auto& el : j) { conf = try_obj(el); if (conf > 0.0) break; }
            }
            if (conf > 0.0) return conf;
        } catch (...) {}
    }
    return 0.0;
}

// ── OpenGraph extraction ──────────────────────────────────────────────────────

static std::unordered_map<std::string, std::string> extract_og(const std::string& html) {
    // OG tags are always in <head> — restrict search region for performance
    auto head_end = html.find("</head>");
    const std::string& region = (head_end != std::string::npos) ? html.substr(0, head_end) : html;

    std::unordered_map<std::string, std::string> og;
    // property before content
    std::regex re1(R"(<meta\s[^>]*property=["\']og:([^"\']+)["\'][^>]*content=["\']([^"\']*)["\'])",
                   std::regex::icase);
    for (auto it = std::sregex_iterator(region.begin(), region.end(), re1);
         it != std::sregex_iterator(); ++it)
        og[(*it)[1].str()] = (*it)[2].str();
    // content before property
    std::regex re2(R"(<meta\s[^>]*content=["\']([^"\']*)["\'][^>]*property=["\']og:([^"\']+)["\'])",
                   std::regex::icase);
    for (auto it = std::sregex_iterator(region.begin(), region.end(), re2);
         it != std::sregex_iterator(); ++it)
        og[(*it)[2].str()] = (*it)[1].str();
    return og;
}

static double fill_from_og(const std::unordered_map<std::string, std::string>& og, Product& p) {
    auto get = [&](const std::string& k) -> std::string {
        auto it = og.find(k); return it != og.end() ? it->second : "";
    };
    auto type = str_lower(get("type"));
    bool is_product_og = (type == "product" || type == "og:product"
                          || type.find("product") != std::string::npos);
    if (!is_product_og) return 0.0;

    if (p.name.empty()) p.name = get("title");
    if (p.images.empty()) { auto img = get("image"); if (!img.empty()) p.images.push_back(img); }
    if (p.price_amount == 0.0) {
        auto amt = get("price:amount");
        if (!amt.empty()) { try { p.price_amount = std::stod(amt); } catch (...) {} }
    }
    if (p.price_currency == "USD") {
        auto cur = get("price:currency");
        if (!cur.empty()) p.price_currency = cur;
    }

    int filled = (!p.name.empty() ? 1 : 0) + (p.price_amount > 0.0 ? 1 : 0) + (!p.images.empty() ? 1 : 0);
    return 0.3 + 0.07 * static_cast<double>(filled);  // 0.3–0.51
}

// ── Heuristic price signal (low confidence, fills gap only) ──────────────────

static bool has_heuristic_price(const std::string& html) {
    std::regex price_re(R"([\$£€₹¥]\s*\d{1,6}(?:[,\d]*)?(?:\.\d{2})?)");
    return std::regex_search(html, price_re);
}

// ── Main pipeline ─────────────────────────────────────────────────────────────

ExtractionResult extract_product(const std::string& html, const std::string& url) {
    Product p;
    p.source_url     = url;
    p.source_domain  = domain_from_url(url);
    p.extracted_at   = iso8601_now();
    p.price_currency = "USD";

    double json_ld_conf = fill_from_json_ld(html, p);
    auto   og           = extract_og(html);
    double og_conf      = fill_from_og(og, p);

    double confidence = std::max(json_ld_conf, og_conf);
    // heuristic price alone is insufficient to clear the gate; used only for future scoring
    if (confidence == 0.0 && has_heuristic_price(html)) confidence = 0.2;

    // T07: graceful degradation — emit nothing below 0.5
    if (confidence < 0.5) return {0.0, std::nullopt};

    if (p.images.size() > 5)    p.images.resize(5);
    if (p.key_specs.size() > 5) p.key_specs.resize(5);

    return {confidence, p};
}

// ── Serialization ─────────────────────────────────────────────────────────────

std::string to_json(const ExtractionResult& result) {
    json j;
    j["confidence"] = result.confidence;
    if (result.product) {
        const auto& p = *result.product;
        j["product"] = {
            {"name",          p.name},
            {"brand",         p.brand},
            {"price",         {{"amount", p.price_amount}, {"currency", p.price_currency}}},
            {"rating",        {{"score",  p.rating_score},  {"count",    p.rating_count}}},
            {"availability",  p.availability.empty() ? "unknown" : p.availability},
            {"images",        p.images},
            {"key_specs",     p.key_specs},
            {"source_url",    p.source_url},
            {"source_domain", p.source_domain},
            {"extracted_at",  p.extracted_at},
        };
    }
    return j.dump();
}

}  // namespace drift::intelligence
