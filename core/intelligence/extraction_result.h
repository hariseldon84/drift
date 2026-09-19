#pragma once
#include <optional>
#include <string>
#include <vector>

namespace drift::intelligence {

struct Product {
    std::string name;
    std::string brand;
    double      price_amount    = 0.0;
    std::string price_currency  = "USD";
    double      rating_score    = 0.0;  // 0–5
    int         rating_count    = 0;
    std::string availability;           // "in_stock" | "out_of_stock" | "limited" | ""
    std::vector<std::string> images;    // capped at 5
    std::vector<std::string> key_specs; // capped at 5
    std::string source_url;
    std::string source_domain;
    std::string extracted_at;           // ISO 8601
};

struct ExtractionResult {
    double confidence = 0.0;
    std::optional<Product> product;  // absent when confidence < 0.5
};

}  // namespace drift::intelligence
