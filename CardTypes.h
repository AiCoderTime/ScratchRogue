#pragma once

enum class CardType {
	Numeric, Lucky, Gilded, Risky,
	Royal, Tricky, Volatile, Experimental
};

inline const char* toString(CardType type) {
	switch (type) {
	case CardType::Numeric:
		return "Numeric";
	case CardType::Lucky:
		return "Lucky";
	case CardType::Gilded:
		return "Gilded";
	case CardType::Risky:
		return "Risky";
	case CardType::Royal:
		return "Royal";
	case CardType::Tricky:
		return "Tricky";
	case CardType::Volatile:
		return "Volatile";
	case CardType::Experimental:
		return "Experimental";
	default:
		return "Unknown";
	}
}