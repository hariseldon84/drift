#pragma once
#include "extraction_result.h"
#include <string>

namespace drift::intelligence {

// Parse raw HTML and URL through the recognition pipeline.
// Returns ExtractionResult; product is absent when confidence < 0.5 (T07 gate).
// ponytail: rule-based classifier; upgrade to Core ML model when signal quality demands it
ExtractionResult extract_product(const std::string& html, const std::string& url);

// Serialize result to JSON string for ExtractionCompletedEvent.normalized_json.
std::string to_json(const ExtractionResult& result);

}  // namespace drift::intelligence
