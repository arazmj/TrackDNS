//
// Created by Amir Razmjou on 12/7/16.
//

#include "Persistence.h"

#include <string>
#include "mysql++/mysql++.h"

Persistence::Persistence(const std::string &db_name, const std::string &db_host, const std::string &username,
                         const std::string &password) {

    if (!connection_.connect(db_name.c_str(), db_host.c_str(),
                      username.c_str(), password.c_str())) {
        throw std::runtime_error { std::string("DB connection failed: ") +
                                           std::string(connection_.error()) };
    }
}

void Persistence::SaveDomain(const Domain &d) {
    mysqlpp::Query update(&connection_);

    update << "INSERT INTO Stats "
           <<    "(dns_name, time_average, count, time_deviation, first_timestamp, last_timestamp) "
           << "VALUES "
           <<    "('"
           <<      d.dns_name()      << "'," << d.time_average()    << "," <<  d.count() << ","
           <<      d.time_deviation() << "," << d.first_timestamp() << "," <<  d.last_timestamp() << ") "
           << "ON DUPLICATE KEY UPDATE "
           <<    "time_average = VALUES(time_average), "
           <<    "count = VALUES(count), "
           <<    "time_deviation = VALUES(time_deviation), "
           <<    "last_timestamp = VALUES(last_timestamp), "
           <<    "first_timestamp = VALUES(first_timestamp)";

    update.exec();
}

void Persistence::LoadDomain(Domain &d) {
    mysqlpp::Query query(&connection_);

    query << "SELECT * FROM stats WHERE dns_name = '" << d.dns_name() << "'";

    auto result = query.store();

    if (!result.empty()) {
        d.time_average(result[0]["time_average"]);
        d.count(result[0]["count"]);
        d.time_deviation(result[0]["time_deviation"]);
        d.first_timestamp(result[0]["first_timestamp"]);
        d.last_timestamp(result[0]["last_timestamp"]);
    }
}

Persistence::~Persistence() {
    //TODO do we need clean up?
}

