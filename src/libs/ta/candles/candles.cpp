#include "candles.hpp"
#include <cstddef>
#include <iostream>
#include <fstream>
#include <cstdlib> 
#include "../../core/config/config.hpp"
#include "../../core/pubsub/pubsub.hpp"
#include "../../utils/string_utils.hpp"
#include "../../utils/file_utils.hpp"



std::ostream& operator<<(std::ostream& os, const Candle& candle) {
    os << "Candle(t=" << candle.t << ", o=" << candle.o << ", h=" << candle.h << ", l=" << candle.l << ", c=" << candle.c << ", vwap=" << candle.vwap << ", n=" << candle.n << ", v=" << candle.v << ", vs=" << candle.vs << ", vb=" << candle.vb << ", q=" << candle.q << ", qs=" << candle.qs << ", qb=" << candle.qb << ")";
    return os;
}

// Candle methods

Candle::Candle(size_t timeframe) {
    this->timeframe = timeframe;
}

Candle::Candle(const Trade& trade, size_t timeframe) {
    this->timeframe = timeframe;
    this->t = trade.t - (trade.t % (timeframe * 1000));
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
    this->qs = trade.is_buyer_maker ? trade.q : 0;
    this->qb = trade.is_buyer_maker ? 0 : trade.q;
}

Candle::Candle(const Candle& candle, size_t timeframe) {
    if (candle.timeframe > timeframe || (timeframe % candle.timeframe) != 0) {
        std::cerr << "Error: Cannot create higher timeframe candle from lower timeframe candle." << std::endl;
        std::exit(EXIT_FAILURE);
    }
    this->timeframe = timeframe;
    this->t = candle.t - (candle.t % (timeframe * 1000));
    this->o = candle.o;
    this->h = candle.h;
    this->l = candle.l;
    this->c = candle.c;
    this->vwap = candle.vwap;
    this->n = candle.n;
    this->v = candle.v;
    this->vs = candle.vs;
    this->vb = candle.vb;
    this->q = candle.q;
    this->qs = candle.qs;
    this->qb = candle.qb;
}

Candle::Candle(const char* buffer) {
    size_t offset = 0;

    std::memcpy(&this->timeframe, buffer + offset, sizeof(this->timeframe));
    offset += sizeof(this->timeframe);

    std::memcpy(&this->t, buffer + offset, sizeof(this->t));
    offset += sizeof(this->t);

    std::memcpy(&this->o, buffer + offset, sizeof(this->o));
    offset += sizeof(this->o);

    std::memcpy(&this->h, buffer + offset, sizeof(this->h));
    offset += sizeof(this->h);

    std::memcpy(&this->l, buffer + offset, sizeof(this->l));
    offset += sizeof(this->l);

    std::memcpy(&this->c, buffer + offset, sizeof(this->c));
    offset += sizeof(this->c);

    std::memcpy(&this->vwap, buffer + offset, sizeof(this->vwap));
    offset += sizeof(this->vwap);

    std::memcpy(&this->n, buffer + offset, sizeof(this->n));
    offset += sizeof(this->n);

    std::memcpy(&this->v, buffer + offset, sizeof(this->v));
    offset += sizeof(this->v);

    std::memcpy(&this->vs, buffer + offset, sizeof(this->vs));
    offset += sizeof(this->vs);

    std::memcpy(&this->vb, buffer + offset, sizeof(this->vb));
    offset += sizeof(this->vb);

    std::memcpy(&this->q, buffer + offset, sizeof(this->q));
    offset += sizeof(this->q);

    std::memcpy(&this->qs, buffer + offset, sizeof(this->qs));
    offset += sizeof(this->qs);

    std::memcpy(&this->qb, buffer + offset, sizeof(this->qb));
}

void Candle::push(const Trade& trade) {
    if (trade.t < this->t || trade.t >= this->t + this->timeframe * 1000) {
        std::cerr << "Error: Trade time is out of candle time range." << std::endl;
        return;
    }
    this->h = this->h > trade.p ? this->h : trade.p;
    this->l = this->l < trade.p ? this->l : trade.p;
    this->c = trade.p;
    this->v += trade.v;
    this->vs += trade.is_buyer_maker ? trade.v : 0;
    this->vb += trade.is_buyer_maker ? 0 : trade.v;
    this->qs += trade.is_buyer_maker ? trade.q : 0;
    this->qb += trade.is_buyer_maker ? 0 : trade.q;
    this->q += trade.q;
    this->n += 1;
    this->vwap = this->v != 0 ? this->q / this->v : 0;
}

void Candle::push(const Candle& candle) {
    if (candle.t < this->t || candle.t >= this->t + this->timeframe * 1000) {
        std::cerr << "Error: Candle time is out of candle time range." << std::endl;
        return;
    }
    this->h = this->h > candle.h ? this->h : candle.h;
    this->l = this->l < candle.l ? this->l : candle.l;
    this->c = candle.c;
    this->v += candle.v;
    this->vs += candle.vs;
    this->vb += candle.vb;
    this->qs += candle.qs;
    this->qb += candle.qb;
    this->q += candle.q;
    this->n += candle.n;
    this->vwap = this->v != 0 ? this->q / this->v : 0;
}

size_t Candle::candle_end_time() {
    return this->t + this->timeframe * 1000 - 1;
} 

bool Candle::is_time_in_candle(size_t t) {
    return t >= this->t && t <= this->candle_end_time();
}

bool Candle::is_time_before_candle(size_t t) {
    return t < this->t;
}

bool Candle::is_time_next_candle(size_t t) {
    return t >= this->t + this->timeframe * 1000 && t < this->t + (this->timeframe * 2) * 1000;
}

Candle Candle::filling_candle() {
    Candle candle(this->timeframe);
    candle.timeframe = this->timeframe;
    candle.t = this->t + this->timeframe * 1000;
    candle.o = this->c;
    candle.h = this->c;
    candle.l = this->c;
    candle.c = this->c;
    candle.vwap = this->vwap;
    candle.n = 0;
    candle.v = 0;
    candle.vs = 0;
    candle.vb = 0;
    candle.q = 0;
    candle.qs = 0;
    candle.qb = 0;
    return candle;
}

void Candle::to_buffer(char* buffer) const {
    size_t offset = 0;

    std::memcpy(buffer + offset, &this->timeframe, sizeof(this->timeframe));
    offset += sizeof(this->timeframe);

    std::memcpy(buffer + offset, &this->t, sizeof(this->t));
    offset += sizeof(this->t);

    std::memcpy(buffer + offset, &this->o, sizeof(this->o));
    offset += sizeof(this->o);

    std::memcpy(buffer + offset, &this->h, sizeof(this->h));
    offset += sizeof(this->h);

    std::memcpy(buffer + offset, &this->l, sizeof(this->l));
    offset += sizeof(this->l);

    std::memcpy(buffer + offset, &this->c, sizeof(this->c));
    offset += sizeof(this->c);

    std::memcpy(buffer + offset, &this->vwap, sizeof(this->vwap));
    offset += sizeof(this->vwap);

    std::memcpy(buffer + offset, &this->n, sizeof(this->n));
    offset += sizeof(this->n);

    std::memcpy(buffer + offset, &this->v, sizeof(this->v));
    offset += sizeof(this->v);

    std::memcpy(buffer + offset, &this->vs, sizeof(this->vs));
    offset += sizeof(this->vs);

    std::memcpy(buffer + offset, &this->vb, sizeof(this->vb));
    offset += sizeof(this->vb);

    std::memcpy(buffer + offset, &this->q, sizeof(this->q));
    offset += sizeof(this->q);

    std::memcpy(buffer + offset, &this->qs, sizeof(this->qs));
    offset += sizeof(this->qs);

    std::memcpy(buffer + offset, &this->qb, sizeof(this->qb));
}

// CandleWriter methods

CandleWriter::CandleWriter(string symbol, size_t timeframe) : candle(timeframe) {
    this->symbol = symbol;
    this->timeframe = timeframe;
    this->file_path_name = (Config::getInstance()).data_path + "um/candles/" + utils::toLowerCase(symbol) + "_" + std::to_string(timeframe) + "s.candles";
    this->file_stream.open(this->file_path_name, std::ios::out | std::ios::binary);
    if (!this->file_stream.is_open()) {
        std::cout << "Error: Could not open file for writing candles: " << this->file_path_name << std::endl;
        std::exit(EXIT_FAILURE);
    }
}

CandleWriter::~CandleWriter() {
    if (this->file_stream.is_open()) {
        this->file_stream.close();
    }
}

void CandleWriter::write_current_candle_to_file() {
    this->candle.to_buffer(this->buffer);
    this->file_stream.write(this->buffer, Candle::buffer_size());
}

void CandleWriter::push(const Trade& trade) {
    if (trade.t < this->last_trade_time) {
        std::cerr << "Error: New trade time is before the last trade time." << std::endl;
        return;
    }

    if (this->candle.n == 0) {
        this->candle = Candle(trade, this->timeframe);
    }
    else if (this->candle.is_time_in_candle(trade.t)) {
        this->candle.push(trade);
    }
    else if (this->candle.is_time_before_candle(trade.t)) {
        std::cerr << "Error: New trade time is before the current candle time." << std::endl;
    }
    else {
        // current trade is after the current candle time. maybe next or maybe more
        // write current candle to file
        this->write_current_candle_to_file();
        // fill gaps with empty candles
        while (!(this->candle.is_time_next_candle(trade.t))) {
            this->candle = this->candle.filling_candle();
            this->write_current_candle_to_file();
        }
        // create new candle with current trade
        this->candle = Candle(trade, this->timeframe);
    }

    this->last_trade_time = trade.t;
}

string candle_file_path_name(string symbol, size_t timeframe) {
    return (Config::getInstance()).data_path + "um/candles/" + utils::toLowerCase(symbol) + "_" + std::to_string(timeframe) + "s.candles";
}

// candle Reader methods

CandleReader::CandleReader(string symbol, size_t timeframe) : current_candle(timeframe) {
    this->timeframe = timeframe;
    this->file_path_name = candle_file_path_name(symbol, timeframe);
    this->sizeof_candle = Candle::buffer_size();
    this->size = utils::get_file_size(this->file_path_name) / this->sizeof_candle;
    this->open();
    this->start = 0;
    this->end = this->size - 1;
}

CandleReader::~CandleReader() {
    this->close();
}

void CandleReader::open() {
    this->file_stream.open(this->file_path_name, std::ios::in | std::ios::binary);
    if (!this->file_stream.is_open()) {
        std::cout << "Error: Could not open file for reading candles: " << this->file_path_name << std::endl;
        std::exit(EXIT_FAILURE);
    }
    this->index = 0;
    // read first candle to initialize first_ts
    if (this->read_next(this->current_candle)) {
        this->first_ts = this->current_candle.t;
        this->last_ts = this->current_candle.t + (this->size - 1) * this->timeframe * 1000;
    }
    // go to start
    this->go(this->start);
}

void CandleReader::close() {
    if (this->file_stream.is_open()) {
        this->file_stream.close();
    }
}

void CandleReader::go(size_t index) {
    if (index >= this->size || index < 0) {
        std::cout << "Error: Index out of range in CandleReader::go(): " << index << std::endl;
        return;
    }
    if (!this->file_stream.is_open()) {
        std::cout << "Error: File is not open for reading candles: " << this->file_path_name << std::endl;
        return;
    }
    this->file_stream.seekg(index * this->sizeof_candle, std::ios::beg);
    this->index = index;
}

void CandleReader::go_to_timestamp(size_t timestamp) {
    if (timestamp < this->first_ts || timestamp > this->last_ts) {
        std::cout << "Error: Timestamp out of range in CandleReader::go_to_timestamp(): " << timestamp << std::endl;
        return;
    }
    size_t index = (timestamp - this->first_ts) / (this->timeframe * 1000);
    this->go(index);
}

bool CandleReader::read_next(Candle& candle) {
    if (this->index > this->end) {
        return false;
    }
    if (!this->file_stream.is_open()) {
        std::cout << "Error: File is not open for reading candles: " << this->file_path_name << std::endl;
        return false;
    }
    if (this->index >= this->size) {
        return false;
    }
    char buffer[this->sizeof_candle];
    this->file_stream.read(buffer, this->sizeof_candle);
    if (this->file_stream.gcount() != this->sizeof_candle) {
        return false;
    }
    candle = Candle(buffer);
    this->index += 1;
    return true;
}

bool CandleReader::read_next() {
    return this->read_next(this->current_candle);
}

bool CandleReader::read(Candle& candle, size_t index) {
    this->go(index);
    return this->read_next(candle);
}

void CandleReader::set_start(size_t start_index) {
    if (start_index >= this->size) {
        std::cout << "Error: Start index out of range in CandleReader::set_start(): " << start_index << std::endl;
        return;
    }
    this->start = start_index;
    this->go(this->start);
}

void CandleReader::set_end(size_t end_index) {
    if (end_index >= this->size) {
        std::cout << "Error: End index out of range in CandleReader::set_end(): " << end_index << std::endl;
        return;
    }
    this->end = end_index;
}

size_t CandleReader::ts_to_index(size_t timestamp) {
    if (timestamp < this->first_ts) {
        return 0;
    }
    if (timestamp > this->last_ts) {
        return this->size - 1;
    }
    return (timestamp - this->first_ts) / (this->timeframe * 1000);
}

void CandleReader::publish(size_t start_ts, size_t end_ts) {
    if (start_ts == 0) start_ts = this->first_ts;
    if (end_ts == 0) end_ts = this->last_ts;
    PubSub& pubsub = PubSub::getInstance();
    this->start = this->ts_to_index(start_ts);
    this->end = this->ts_to_index(end_ts);
    this->go(this->start);
    while (this->index <= this->end) {
        this->read_next();
        pubsub.publish("candle", (void*)&this->current_candle);
    }
    pubsub.publish("candle_end", nullptr);
}

// Candles methods

Candles::Candles(size_t timeframe, size_t retain_count) : boost::circular_buffer<Candle>(retain_count) {
    this->timeframe = timeframe;
    this->topic_candles_on_new_candle = "candle_" + std::to_string(timeframe);
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
        this->push_back(Candle(candle, this->timeframe));
    }
    else if (this->back().is_time_before_candle(candle.t)) {
        std::cerr << "Error: New candle time is before the last candle time." << std::endl;
    }
    else if (this->back().is_time_in_candle(candle.t)) {
        this->back().push(candle);
    }
    else {
        // fill gaps with empty candles
        while (!(this->back().is_time_next_candle(candle.t))) {
            this->pubsub.publish(this->topic_candles_on_new_candle, (void*)&this->back());
            this->push_back(this->back().filling_candle());
        }
        this->pubsub.publish(this->topic_candles_on_new_candle, (void*)&this->back());
        this->push_back(Candle(candle, this->timeframe));
    }
}

void Candles::push(const Trade& trade) {
    if (this->empty()) {
        this->push_back(Candle(trade, this->timeframe));
    }
    else if (this->back().is_time_before_candle(trade.t)) {
        std::cerr << "Error: New trade time is before the last candle time." << std::endl;
    }
    else if (this->back().is_time_in_candle(trade.t)) {
        this->back().push(trade);
    }
    else {
        // fill gaps with empty candles
        while (!(this->back().is_time_next_candle(trade.t))) {
            this->pubsub.publish(this->topic_candles_on_new_candle, (void*)&this->back());
            this->push_back(this->back().filling_candle());
        }
        this->pubsub.publish(this->topic_candles_on_new_candle, (void*)&this->back());
        this->push_back(Candle(trade, this->timeframe));
    }
}

// CandlesVector methods

CandlesVector::CandlesVector(size_t timeframe) : timeframe(timeframe) {}

CandlesVector::CandlesVector(string symbol, size_t start_ts, size_t end_ts) : timeframe(1) {
    this->read_from_binary_file(symbol, start_ts, end_ts);
}

void CandlesVector::push(const Candle& candle) {
    if (this->empty()) {
        this->emplace_back(Candle(candle, this->timeframe));
    }
    else if (this->back().is_time_before_candle(candle.t)) {
        std::cerr << "Error: New candle time is before the last candle time." << std::endl;
    }
    else if (this->back().is_time_in_candle(candle.t)) {
        this->back().push(candle);
    }
    else {
        // fill gaps with empty candles
        while (!(this->back().is_time_next_candle(candle.t))) {
            this->emplace_back(this->back().filling_candle());
        }
        this->emplace_back(Candle(candle, this->timeframe));
    }
}

void CandlesVector::push(const Trade& trade) {
    if (this->empty()) {
        this->emplace_back(Candle(trade, this->timeframe));
    }
    else if (this->back().is_time_before_candle(trade.t)) {
        std::cerr << "Error: New trade time is before the last candle time." << std::endl;
    }
    else if (this->back().is_time_in_candle(trade.t)) {
        this->back().push(trade);
    }
    else {
        // fill gaps with empty candles
        while (!(this->back().is_time_next_candle(trade.t))) {
            this->emplace_back(this->back().filling_candle());
        }
        this->emplace_back(Candle(trade, this->timeframe));
    }
}

void CandlesVector::build_from_trade_vector(const std::vector<Trade>& trades) {
    for (const auto& trade : trades) {
        this->push(trade);
    }
}


void CandlesVector::write_to_binary_file(const std::string& file_path_name, string mode) {
    std::ofstream candle_data(file_path_name, std::ios::out | std::ios::binary); // Open the binary file for writing
    if (!candle_data.is_open()) {
        std::cout << "Error: Could not open file for writing candles: " << file_path_name << std::endl;
        return; // Return early if the file cannot be opened
    }
    for (const auto& candle : *this) {
        // write fields one by one to ensure no padding issues
        if (mode == "full") {
            // candle_data.write(reinterpret_cast<const char*>(&candle.timeframe), sizeof(candle.timeframe));
            candle_data.write(reinterpret_cast<const char*>(&candle.t), sizeof(candle.t));
            candle_data.write(reinterpret_cast<const char*>(&candle.o), sizeof(candle.o));
            candle_data.write(reinterpret_cast<const char*>(&candle.h), sizeof(candle.h));
            candle_data.write(reinterpret_cast<const char*>(&candle.l), sizeof(candle.l));
            candle_data.write(reinterpret_cast<const char*>(&candle.c), sizeof(candle.c));
            candle_data.write(reinterpret_cast<const char*>(&candle.vwap), sizeof(candle.vwap));
            candle_data.write(reinterpret_cast<const char*>(&candle.n), sizeof(candle.n));
            candle_data.write(reinterpret_cast<const char*>(&candle.v), sizeof(candle.v));
            candle_data.write(reinterpret_cast<const char*>(&candle.vs), sizeof(candle.vs));
            candle_data.write(reinterpret_cast<const char*>(&candle.vb), sizeof(candle.vb));
            candle_data.write(reinterpret_cast<const char*>(&candle.q), sizeof(candle.q));
            candle_data.write(reinterpret_cast<const char*>(&candle.qs), sizeof(candle.qs));
            candle_data.write(reinterpret_cast<const char*>(&candle.qb), sizeof(candle.qb));
        }
        else if (mode == "tohlcv") {
            candle_data.write(reinterpret_cast<const char*>(&candle.t), sizeof(candle.t));
            candle_data.write(reinterpret_cast<const char*>(&candle.o), sizeof(candle.o));
            candle_data.write(reinterpret_cast<const char*>(&candle.h), sizeof(candle.h));
            candle_data.write(reinterpret_cast<const char*>(&candle.l), sizeof(candle.l));
            candle_data.write(reinterpret_cast<const char*>(&candle.c), sizeof(candle.c));
            candle_data.write(reinterpret_cast<const char*>(&candle.v), sizeof(candle.v));
        }
        else if (mode == "thlpv") {
            // t,h,l,vwap,v
            candle_data.write(reinterpret_cast<const char*>(&candle.t), sizeof(candle.t));
            candle_data.write(reinterpret_cast<const char*>(&candle.h), sizeof(candle.h));
            candle_data.write(reinterpret_cast<const char*>(&candle.l), sizeof(candle.l));
            candle_data.write(reinterpret_cast<const char*>(&candle.vwap), sizeof(candle.vwap));
            candle_data.write(reinterpret_cast<const char*>(&candle.v), sizeof(candle.v));
        }
        else if (mode == "tpv") {
            // t, vwap, v
            candle_data.write(reinterpret_cast<const char*>(&candle.t), sizeof(candle.t));
            candle_data.write(reinterpret_cast<const char*>(&candle.vwap), sizeof(candle.vwap));
            candle_data.write(reinterpret_cast<const char*>(&candle.v), sizeof(candle.v));
        }
    }
    candle_data.close(); // Close the file after writing
}

void CandlesVector::report_candles_integrity(const std::string& file_path_name, size_t start_ts, size_t end_ts) {
    cout << "Reporting candles integrity for file: " << file_path_name << std::endl;
    double eps = 1e-5;
    this->read_from_binary_file(file_path_name, start_ts, end_ts);
    Candle previous_candle = this->front();
    double total_v = 0;
    double total_vs = 0;
    double total_vb = 0;
    double total_q = 0;
    double total_qs = 0;
    double total_qb = 0;
    size_t count = 0;
    for (const auto& candle : *(this)) {
        count++;
        if (count == 1) continue;

        if (candle.t != previous_candle.t + this->timeframe * 1000) { std::cout << "Gap detected between candles at " << previous_candle.t << " and " << candle << std::endl; }
        if (candle.o < 0 || candle.h < 0 || candle.l < 0 || candle.c < 0) { std::cout << "Negative price detected in candle at " << candle << std::endl; }
        if (candle.v < 0 || candle.vs < 0 || candle.vb < 0) { std::cout << "Negative volume detected in candle at " << candle << std::endl; }
        if (candle.q < 0 || candle.qs < 0 || candle.qb < 0) { std::cout << "Negative quantity detected in candle at " << candle << std::endl; }
        if (candle.vwap < 0) { std::cout << "Negative VWAP detected in candle at " << candle << std::endl; }
        if (candle.n < 0) { std::cout << "Negative trade count detected in candle at " << candle << std::endl; }
        if (candle.h - candle.l < -eps || candle.h - candle.o < -eps || candle.h - candle.c < -eps || (candle.n > 0 && candle.h - candle.vwap < -eps)) { std::cout << "Inconsistent high price detected in candle at " << candle << std::endl; }
        if (candle.l - candle.h > eps || candle.l - candle.o > eps || candle.l - candle.c > eps || (candle.n > 0 && candle.l - candle.vwap > eps)) { std::cout << "Inconsistent low price detected in candle at " << candle << std::endl; }
        if (candle.q - candle.qs - candle.qb > eps) { std::cout << "Inconsistent quantity detected in candle at " << candle << std::endl; }
        if (candle.v - candle.vs - candle.vb > eps) { std::cout << "Inconsistent volume detected in candle at " << candle << std::endl; }
        if (candle.n > 0 && abs(candle.vwap - (candle.q / candle.v)) > eps) { std::cout << "Inconsistent VWAP detected in candle at " << candle << std::endl; }

        total_v += candle.v;
        total_vs += candle.vs;
        total_vb += candle.vb;
        total_q += candle.q;
        total_qs += candle.qs;
        total_qb += candle.qb;

        previous_candle = candle;
    }

    // eps = count * 1e-7;
    eps = 1e-1;

    if (abs(total_v - (total_vs + total_vb)) > eps) { std::cout << "Total volume inconsistency detected. amount is: " << total_v - (total_vs + total_vb) << std::endl; }
    if (abs(total_q - (total_qs + total_qb)) > eps) { std::cout << "Total quantity inconsistency detected. amount is: " << total_q - (total_qs + total_qb) << std::endl; }

    // std::cout << "Total volume: " << total_v << " = " << total_vs << " + " << total_vb << std::endl;
    // std::cout << "Total quantity: " << total_q << " = " << total_qs << " + " << total_qb << std::endl;
    cout << "------------------------------------------------------------" << std::endl;
}

void CandlesVector::read_from_binary_file(const std::string& symbol, size_t start_ts, size_t end_ts) {
    this->clear();
    CandleReader reader(symbol);
    if (start_ts == 0) start_ts = reader.first_ts;
    if (end_ts == 0) end_ts = reader.last_ts;
    reader.go_to_timestamp(start_ts);
    while (reader.read_next()) {
        if (reader.current_candle.t > end_ts) break;
        this->push(reader.current_candle);
    }   
}

