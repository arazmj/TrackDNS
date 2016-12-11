//
// Created by Amir Razmjou on 12/6/16.
//

#include <random>
#include <iostream>
#include <tuple>
#include <iomanip>
#include <cmath>
#include <sstream>

#include "Domain.h"
#include "ldns/ldns.h"

std::string get_random_str()
{
    static const char alphabet[] =
            "abcdefghijklmnopqrstuvwxyz"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                    "0123456789";

    //TODO make string size random
    static const size_t S_LEN = 20;

    //TODO make the random device randome engine etc member variable or even global static
    std::random_device rd;
    std::default_random_engine rng(rd());
    std::uniform_int_distribution<> dist(0,sizeof(alphabet)/sizeof(*alphabet)-2);

    std::string str;
    str.reserve(S_LEN);
    std::generate_n(std::back_inserter(str), S_LEN, [&]() { return alphabet[dist(rng)];});

    return str;
}

std::tuple<querytime, timestamp> Domain::process(const std::string &url) const {
    //TODO can we store ldns_resolver per class so we won't need to instantiate it every time.
    //TODO or maybe even we should have only a single resolver per app
    auto dname = ldns_dname_new_frm_str(url.c_str());

    if (!dname)
        throw std::runtime_error { "ldns_dname_new_frm_str failure" };

    ldns_resolver *resolver { nullptr };

    if (ldns_resolver_new_frm_file(&resolver, nullptr) != LDNS_STATUS_OK)
        throw std::runtime_error { "ldns_resolver_new_frm_file failure failure" };

    auto packet = ldns_resolver_query(resolver, dname, LDNS_RR_TYPE_A, LDNS_RR_CLASS_IN, LDNS_RD);

    //TODO not efficent
    ldns_rdf_deep_free(dname);

    if (!packet)
        throw std::runtime_error { "ldns_resolver_query failure" };

    querytime qt = ldns_pkt_querytime(packet);
    timestamp ts = (timestamp) packet->timestamp.tv_usec;
    return std::make_tuple(qt, ts);
}

void Domain::Update() {
    /* append random string to begining of the dns_name url */
    std::string random_url = get_random_str() + "." + domain_;

    /* update query results */
    querytime new_querytime;
    std::tie(new_querytime, last_timestamp_) = process(random_url);

    /* update first time stamp if it is not set */
    if (first_timestamp_ == 0) {
        first_timestamp_.store(last_timestamp_);
    }

    /* do the calculation */
    count_++;

    querytime old_average = (querytime) time_average_;
    time_average_ = ((count_ * time_average_) + new_querytime) / count_ + 1;

    //TODO needs more work
    if (count_ == 1) {
        time_deviation_ = 0;
    }
    else if (count_ == 2)
    {
        time_deviation_ = std::sqrt(pow(time_deviation_ - time_average_, 2.0) +
                                            pow(last_querytime_ - time_average_, 2.0)) / count_;
    }
    else if (count_ > 2)
    {
        time_deviation_ = std::sqrt(((count_ - 2) * pow(time_deviation_, 2.0) +
                (last_querytime_ - last_querytime_) * (last_querytime_ - old_average)) / (count_ - 1));
    }


    last_querytime_ = new_querytime;
}

std::string Domain::dns_name() const {
    return domain_;
}

double Domain::time_average() const {
    return time_average_.load();
}

double Domain::time_deviation() const {
    return time_deviation_.load();
}

uint32_t Domain::last_querytime() const {
    return last_querytime_.load();
}

uint32_t Domain::count() const {
    return count_.load();
}

uint32_t Domain::first_timestamp() const {
    return first_timestamp_.load();
}

uint32_t Domain::last_timestamp() const {
    return last_timestamp_.load();
}

void Domain::time_average(double time) {
    time_average_ = time;
}

void Domain::time_deviation(double time) {
    time_deviation_ = time;
}

void Domain::count(uint32_t count) {
    count_ = count;
}

void Domain::last_querytime(querytime time) {
    last_querytime_ = time;
}

void Domain::first_timestamp(uint32_t time) {
    first_timestamp_ = time;
}

void Domain::last_timestamp(uint32_t time) {
    last_timestamp_ = time;
}

std::ostream &operator<<(std::ostream &os, const Domain &domain) {
    return os << std::setw(15) << std::left << domain.dns_name()
              << std::setw(15) << std::left << domain.last_querytime()
              << std::setw(15) << std::left << domain.last_timestamp()
              << std::setw(15) << std::left << domain.first_timestamp()
              << std::setw(15) << std::left << domain.count()
              << std::setw(15) << std::left << domain.time_average()
              << std::setw(15) << std::left << domain.time_deviation();
}

void Domain::ShowHeaders() {
    //TODO not good
    std::cout << std::setw(15) << std::left << "DNS"
              << std::setw(15) << std::left << "Current"
              << std::setw(15) << std::left << "Last TS"
              << std::setw(15) << std::left << "First TS"
              << std::setw(15) << std::left << "Count"
              << std::setw(15) << std::left << "Average"
              << std::setw(15) << std::left << "Deviation"
                                            << std::endl;
}

bool Domain::operator<(const Domain &rhs) const {
    return last_querytime_ < rhs.last_querytime_;
}
