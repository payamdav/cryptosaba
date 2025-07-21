#include "candles.hpp"
#include <iostream>


std::ostream& operator<<(std::ostream& os, const Candle& candle) {
    os << "Candle(t=" << candle.t << ", o=" << candle.o << ", h=" << candle.h << ", l=" << candle.l << ", c=" << candle.c << ", vwap=" << candle.vwap << ", n=" << candle.n << ", v=" << candle.v << ", vs=" << candle.vs << ", vb=" << candle.vb << ", q=" << candle.q << ")";
    return os;
}




// Candle methods

Candle::Candle(const Trade& trade, size_t sec) {
    this->t = trade.t - (trade.t % (sec * 1000));
    this->o = trade.p;
    this->h = trade.p;
    this->l = trade.p;
    this->c = trade.p;
    this->vwap = trade.p;
    this->n = 1;
    this->v = trade.v;
    this->vs = trade.is_buyer_maker ? trade.v : 0;
    this->vb = trade.is_buyer_maker ? 0 : trade.v;
    this->q = trade.q;
}

Candle::Candle(const Candle& candle, size_t sec) {
    this->t = candle.t - (candle.t % (sec * 1000));
    this->o = candle.o;
    this->h = candle.h;
    this->l = candle.l;
    this->c = candle.c;
    this->vwap = candle.vwap;
    this->n = 1;
    this->v = candle.v;
    this->vs = candle.vs;
    this->vb = candle.vb;
    this->q = candle.q;
}

void Candle::push(const Trade& trade, size_t sec) {
    if (trade.t < this->t || trade.t >= this->t + sec * 1000) {
        std::cerr << "Error: Trade time is out of candle time range." << std::endl;
        return;
    }
    this->h = this->h > trade.p ? this->h : trade.p;
    this->l = this->l < trade.p ? this->l : trade.p;
    this->c = trade.p;
    this->v += trade.v;
    this->vs += trade.is_buyer_maker ? trade.v : 0;
    this->vb += trade.is_buyer_maker ? 0 : trade.v;
    this->q += trade.q;
    this->n += 1;
    this->vwap = this->v != 0 ? this->q / this->v : 0;
}

void Candle::push(const Candle& candle, size_t sec) {
    if (candle.t < this->t || candle.t >= this->t + sec * 1000) {
        std::cerr << "Error: Candle time is out of candle time range." << std::endl;
        return;
    }
    this->h = this->h > candle.h ? this->h : candle.h;
    this->l = this->l < candle.l ? this->l : candle.l;
    this->c = candle.c;
    this->v += candle.v;
    this->vs += candle.vs;
    this->vb += candle.vb;
    this->q += candle.q;
    this->n += candle.n;
    this->vwap = this->v != 0 ? this->q / this->v : 0;
}

bool Candle::is_time_in_candle(size_t t, size_t sec) {
    return t >= this->t && t < this->t + sec * 1000;
}

bool Candle::is_time_before_candle(size_t t) {
    return t < this->t;
}

bool Candle::is_time_next_candle(size_t t, size_t sec) {
    return t >= this->t + sec * 1000 && t < this->t + (sec * 2) * 1000;
}

Candle Candle::filling_candle(size_t sec) {
    Candle candle;
    candle.t = this->t + sec * 1000;
    candle.o = this->c;
    candle.h = this->c;
    candle.l = this->c;
    candle.c = this->c;
    candle.vwap = this->c;
    candle.n = 0;
    candle.v = 0;
    candle.vs = 0;
    candle.vb = 0;
    candle.q = 0;
    return candle;
}

// Candles methods

Candles::Candles(size_t sec, size_t retain_count) : boost::circular_buffer<Candle>(retain_count) {
    this->sec = sec;
    this->topic_candles_on_new_candle = "candle_" + std::to_string(sec);
}

Candles * Candles::subscribe_to_pubsub_trades() {
    this->pubsub.subscribe("trade", [this](const void* data) {
        const Trade* trade = static_cast<const Trade*>(data);
        this->push(*trade);
    });
    return this;
}

Candles * Candles::subscribe_to_pubsub_candles() {
    this->pubsub.subscribe("candle_1", [this](const void* data) {
        const Candle* candle = static_cast<const Candle*>(data);
        this->push(*candle);
    });
    return this;
}

Candles * Candles::build_from_1s_candles(const Candles& candles_1s) {
    for (const auto& candle_1s : candles_1s) {
        this->push(candle_1s);
    }
    return this;
}

void Candles::push(const Candle& candle) {
    if (this->empty()) {
        this->push_back(Candle(candle, this->sec));
    }
    else if (this->back().is_time_before_candle(candle.t)) {
        std::cerr << "Error: New candle time is before the last candle time." << std::endl;
    }
    else if (this->back().is_time_in_candle(candle.t, this->sec)) {
        this->back().push(candle, this->sec);
    }
    else {
        // fill gaps with empty candles
        while (!(this->back().is_time_next_candle(candle.t, this->sec))) {
            this->pubsub.publish(this->topic_candles_on_new_candle, (void*)&this->back());
            this->push_back(this->back().filling_candle(this->sec));
        }
        this->pubsub.publish(this->topic_candles_on_new_candle, (void*)&this->back());
        this->push_back(Candle(candle, this->sec));
    }
}

void Candles::push(const Trade& trade) {
    if (this->empty()) {
        this->push_back(Candle(trade, this->sec));
    }
    else if (this->back().is_time_before_candle(trade.t)) {
        std::cerr << "Error: New trade time is before the last candle time." << std::endl;
    }
    else if (this->back().is_time_in_candle(trade.t, this->sec)) {
        this->back().push(trade, this->sec);
    }
    else {
        // fill gaps with empty candles
        while (!(this->back().is_time_next_candle(trade.t, this->sec))) {
            this->pubsub.publish(this->topic_candles_on_new_candle, (void*)&this->back());
            this->push_back(this->back().filling_candle(this->sec));
        }
        this->pubsub.publish(this->topic_candles_on_new_candle, (void*)&this->back());
        this->push_back(Candle(trade, this->sec));
    }
}