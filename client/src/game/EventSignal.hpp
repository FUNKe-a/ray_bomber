#pragma once

#include <algorithm>
#include <functional>
#include <memory>
#include <utility>
#include <vector>

template<typename Event>
class EventSignal
{
public:
    using Callback = std::function<void(const Event&)>;

private:
    struct Slot
    {
        Callback callback;
        bool active = true;
    };

public:
    class Subscription
    {
    public:
        Subscription() = default;

        ~Subscription()
        {
            disconnect();
        }

        Subscription(const Subscription&) = delete;
        Subscription& operator=(const Subscription&) = delete;

        Subscription(Subscription&& other) noexcept
            : slot(std::move(other.slot))
        {
        }

        Subscription& operator=(Subscription&& other) noexcept
        {
            if (this != &other)
            {
                disconnect();
                slot = std::move(other.slot);
            }

            return *this;
        }

        void disconnect() noexcept
        {
            if (const auto current = slot.lock())
            {
                current->active = false;
            }

            slot.reset();
        }

    private:
        friend class EventSignal;

        explicit Subscription(const std::shared_ptr<Slot>& slot)
            : slot(slot)
        {
        }

        std::weak_ptr<Slot> slot;
    };

    EventSignal() = default;

    ~EventSignal()
    {
        disconnectAll();
    }

    EventSignal(const EventSignal&) = delete;
    EventSignal& operator=(const EventSignal&) = delete;

    EventSignal(EventSignal&& other) noexcept
        : slots(std::move(other.slots))
    {
        other.slots.clear();
    }

    EventSignal& operator=(EventSignal&& other) noexcept
    {
        if (this != &other)
        {
            disconnectAll();

            slots = std::move(other.slots);
            other.slots.clear();
        }

        return *this;
    }

    Subscription subscribe(Callback callback)
    {
        if (!callback)
        {
            return {};
        }

        removeInactiveSlots();

        auto slot = std::make_shared<Slot>(
            Slot{std::move(callback)}
        );

        slots.push_back(slot);

        return Subscription{slot};
    }

    void notify(const Event& event)
    {
        removeInactiveSlots();

        // Kopija leidžia callback metu keisti prenumeratas.
        const auto currentSlots = slots;

        for (const auto& slot : currentSlots)
        {
            if (slot->active)
            {
                slot->callback(event);
            }
        }
    }

private:
    void removeInactiveSlots()
    {
        std::erase_if(slots, [](const auto& slot) {
            return !slot->active;
        });
    }

    void disconnectAll() noexcept
    {
        for (const auto& slot : slots)
        {
            slot->active = false;
        }

        slots.clear();
    }

    std::vector<std::shared_ptr<Slot>> slots;
};