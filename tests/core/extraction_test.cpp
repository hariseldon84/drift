#include "intelligence/product_extractor.h"
#include <cassert>
#include <chrono>
#include <cstdio>
#include <string>

using namespace drift::intelligence;

// ── Test helpers ──────────────────────────────────────────────────────────────

static int failures = 0;

#define CHECK(expr) do { \
    if (!(expr)) { std::fprintf(stderr, "FAIL: %s  (%s:%d)\n", #expr, __FILE__, __LINE__); ++failures; } \
} while(0)

// ── Sample HTML fixtures ──────────────────────────────────────────────────────

static const char* kProductJsonLd = R"html(
<!DOCTYPE html><html><head>
<script type="application/ld+json">
{"@context":"https://schema.org","@type":"Product",
 "name":"Test Widget Pro","brand":{"@type":"Brand","name":"Acme Corp"},
 "offers":{"@type":"Offer","price":"29.99","priceCurrency":"USD",
            "availability":"https://schema.org/InStock"},
 "aggregateRating":{"@type":"AggregateRating","ratingValue":"4.5","reviewCount":"1200"},
 "image":"https://example.com/img.jpg",
 "description":"A great widget. Perfect for testing. Works everywhere."}
</script></head><body><h1>Test Widget Pro</h1></body></html>
)html";

static const char* kNonProductPage = R"html(
<!DOCTYPE html><html><head>
<title>News Article: Weather Today</title>
<meta property="og:type" content="article"/>
<meta property="og:title" content="Weather Today"/>
</head><body><p>The weather is sunny today.</p></body></html>
)html";

static const char* kOgProductPage = R"html(
<!DOCTYPE html><html><head>
<meta property="og:type" content="product"/>
<meta property="og:title" content="Blue Running Shoes"/>
<meta property="og:image" content="https://example.com/shoes.jpg"/>
<meta property="og:price:amount" content="89.99"/>
<meta property="og:price:currency" content="USD"/>
</head><body></body></html>
)html";

static const char* kGraphJsonLd = R"html(
<!DOCTYPE html><html><head>
<script type="application/ld+json">
{"@context":"https://schema.org","@graph":[
  {"@type":"WebSite","name":"Shop"},
  {"@type":"Product","name":"Graph Product","offers":{"price":"49.00","priceCurrency":"EUR"}}
]}
</script></head><body></body></html>
)html";

// ── Tests ─────────────────────────────────────────────────────────────────────

static void test_json_ld_product() {
    auto r = extract_product(kProductJsonLd, "https://example.com/product/widget");
    CHECK(r.confidence >= 0.7);
    CHECK(r.product.has_value());
    CHECK(r.product->name == "Test Widget Pro");
    CHECK(r.product->brand == "Acme Corp");
    CHECK(r.product->price_amount == 29.99);
    CHECK(r.product->price_currency == "USD");
    CHECK(r.product->availability == "in_stock");
    CHECK(r.product->rating_score == 4.5);
    CHECK(r.product->rating_count == 1200);
    CHECK(r.product->images.size() == 1);
    CHECK(r.product->source_domain == "example.com");
    CHECK(!r.product->extracted_at.empty());
}

static void test_non_product_page() {
    auto r = extract_product(kNonProductPage, "https://news.example.com/weather");
    CHECK(r.confidence < 0.5);
    CHECK(!r.product.has_value());
}

static void test_og_product() {
    auto r = extract_product(kOgProductPage, "https://shop.example.com/shoes");
    CHECK(r.confidence >= 0.5);
    CHECK(r.product.has_value());
    CHECK(r.product->name == "Blue Running Shoes");
    CHECK(r.product->price_amount == 89.99);
}

static void test_graph_json_ld() {
    auto r = extract_product(kGraphJsonLd, "https://shop.example.com/item");
    CHECK(r.confidence >= 0.5);
    CHECK(r.product.has_value());
    CHECK(r.product->name == "Graph Product");
    CHECK(r.product->price_currency == "EUR");
}

static void test_empty_html() {
    auto r = extract_product("", "https://example.com/");
    CHECK(r.confidence < 0.5);
    CHECK(!r.product.has_value());
}

static void test_to_json() {
    auto r = extract_product(kProductJsonLd, "https://example.com/product/widget");
    auto s = to_json(r);
    CHECK(!s.empty());
    CHECK(s.find("Test Widget Pro") != std::string::npos);
    CHECK(s.find("confidence") != std::string::npos);
}

// T06: pipeline must complete within 500ms on a 200KB page
static void test_performance() {
    std::string big_html = kProductJsonLd;
    big_html.reserve(200'000);
    while (big_html.size() < 200'000)
        big_html += "<p>Lorem ipsum dolor sit amet consectetur adipiscing elit.</p>\n";

    auto t0 = std::chrono::steady_clock::now();
    auto r  = extract_product(big_html, "https://example.com/big");
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                  std::chrono::steady_clock::now() - t0).count();

    CHECK(r.product.has_value());
    CHECK(ms < 500);
    if (ms >= 500)
        std::fprintf(stderr, "  performance: %lldms (budget: 500ms)\n",
                     static_cast<long long>(ms));
}

int main() {
    test_json_ld_product();
    test_non_product_page();
    test_og_product();
    test_graph_json_ld();
    test_empty_html();
    test_to_json();
    test_performance();

    if (failures == 0) {
        std::puts("drift.intelligence: all checks passed");
        return 0;
    }
    std::fprintf(stderr, "drift.intelligence: %d check(s) failed\n", failures);
    return 1;
}
