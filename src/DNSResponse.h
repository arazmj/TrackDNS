//
// Created by Amir Razmjou on 12/4/16.
//

#ifndef TRACKDNS_DNSRESPONSE_H
#define TRACKDNS_DNSRESPONSE_H

#include <string>

using namespace std;

class DNSResponse {
public:
    string dns;
    int ts_start;
    int ts_end;
};


#endif //TRACKDNS_DNSRESPONSE_H
