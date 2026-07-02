#pragma once
#include <string>
#include <memory>

enum class EventType { MARKET, SIGNAL, ORDER, FILL };
enum class SignalType { LONG, EXIT };
enum class OrderType  { MARKET, LIMIT };
enum class Direction  { BUY, SELL };

// Base class for all events flowing through the EventQueue.
struct Event {
    EventType type;
    explicit Event(EventType t) : type(t) {}
    virtual ~Event() = default;
};

// Emitted by DataHandler when a new bar becomes available.
struct MarketEvent : public Event {
    std::string symbol;
    std::string date;
    MarketEvent(std::string sym, std::string d)
        : Event(EventType::MARKET), symbol(std::move(sym)), date(std::move(d)) {}
};

// Emitted by Strategy when it detects a trading opportunity.
struct SignalEvent : public Event {
    std::string symbol;
    std::string date;
    SignalType signalType;
    double strength; // 0..1, reserved for position-sizing logic
    SignalEvent(std::string sym, std::string d, SignalType st, double str = 1.0)
        : Event(EventType::SIGNAL), symbol(std::move(sym)), date(std::move(d)),
          signalType(st), strength(str) {}
};

// Emitted by Portfolio to request an order be placed.
struct OrderEvent : public Event {
    std::string symbol;
    OrderType orderType;
    int quantity;
    Direction direction;
    OrderEvent(std::string sym, OrderType ot, int qty, Direction dir)
        : Event(EventType::ORDER), symbol(std::move(sym)), orderType(ot),
          quantity(qty), direction(dir) {}
};

// Emitted by ExecutionHandler once an order has been "filled".
struct FillEvent : public Event {
    std::string symbol;
    std::string date;
    int quantity;
    Direction direction;
    double fillPrice;
    double commission;
    FillEvent(std::string sym, std::string d, int qty, Direction dir, double price, double comm)
        : Event(EventType::FILL), symbol(std::move(sym)), date(std::move(d)),
          quantity(qty), direction(dir), fillPrice(price), commission(comm) {}
};

using EventPtr = std::shared_ptr<Event>;
