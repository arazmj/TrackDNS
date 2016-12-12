//
// Created by Amir Razmjou on 12/7/16.
//

#ifndef TRACKDNS_PERSISTENCE_H
#define TRACKDNS_PERSISTENCE_H

#include "mysql++/mysql++.h"

#include "Domain.h"

class Persistence {
private:
    mysqlpp::Connection connection_;

public:
    Persistence(const std::string &db_name, const std::string &db_host,
                const std::string &username, const std::string &password);
    void SaveDomain(const Domain &domain);
    void LoadDomain(Domain &domain);
    virtual ~Persistence();
};


#endif //TRACKDNS_PERSISTENCE_H
