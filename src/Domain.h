//
// Created by Amir Razmjou on 12/6/16.
//

#ifndef TRACKDNS_DOMAIN_H
#define TRACKDNS_DOMAIN_H

#include <ostream>
#include <atomic>

using querytime = uint32_t;
using timestamp = uint32_t;

class Domain {


private:
    std::tuple<querytime, timestamp> process(const std::string &url) const;

public:
    Domain(std::string domain): domain_(domain) {}
    Domain(const Domain&) = delete;
    Domain& operator=(const Domain&) = delete;
    Domain(Domain&&) noexcept {}
    Domain& operator=(Domain&&) = default;

public:
    void Update();

private:
    std::string domain_;
    std::atomic<double> time_average_ {0};
    std::atomic<double> time_deviation_ {0};

    std::atomic<uint32_t> count_ {0};
    std::atomic<querytime> last_querytime_ {0};
    std::atomic<timestamp> first_timestamp_ {0};
    std::atomic<timestamp> last_timestamp_ {0};

public:
    std::string domain() const;
    double time_average() const;
    double time_deviation() const;
    uint32_t count() const;
    uint32_t last_querytime() const;
    uint32_t first_timestamp() const;
    uint32_t last_timestamp() const;


    friend std::ostream &operator<<(std::ostream &os, const Domain &domain);
};


#endif //TRACKDNS_DOMAIN_H
