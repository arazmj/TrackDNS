#include <iostream>
#include <string>
#include <thread>
#include <future>
#include <csignal>
#include <tuple>
#include <random>
#include <map>


//#include "cmdline.h"
#include "ldns/ldns.h"
#include "mysql++/mysql++.h"
#include "Queue.h"
#include "DNSResponse.h"
#include "ThreadPool.h"


bool stop_flag = false;
void signal_handler(int signal)
{
    stop_flag = true;
}

template <class R, class P>
void shcedule(const std::chrono::duration<R, P>& duration, std::function<void()> func)
{
    while (!stop_flag) {
        func();
        this_thread::sleep_for(duration);
    }
}

void
query()
{
    ldns_resolver *resolver_;
    ldns_status s = ldns_resolver_new_frm_file(&resolver_, NULL);

    string domain_to_query = "ll.facebook.com";
    ldns_rdf *rdf = ldns_dname_new_frm_str(domain_to_query.c_str());

    if (rdf == NULL) {
        cerr << domain_to_query << " is not a valid domain" << endl;
        return;
    }
    auto start = std::chrono::high_resolution_clock::now();

    ldns_pkt *p = ldns_resolver_query(resolver_, rdf, LDNS_RR_TYPE_A, LDNS_RR_CLASS_IN, LDNS_RD);
    cout << "p->timestamp" << p->timestamp.tv_sec << endl;


    auto end = std::chrono::high_resolution_clock::now();

    if (p != NULL) {
        std::chrono::duration<double> elapsed_seconds =  end-start;
        cout << "This is :" << elapsed_seconds.count() << endl;
        ldns_pkt_free(p);
    } else {
        cerr << domain_to_query << " query failed" << endl;
    }

    ldns_rdf_deep_free(rdf);
}


int get_random_time(const string& domain) {
    int min = 2000;
    int max = 5000 /** domain.length()*/;
    int ms = min + (rand() % (max - min + 1));
    this_thread::sleep_for(chrono::milliseconds(ms));
    return ms;
}

string get_random_str()
{
    static const char alphabet[] =
            "abcdefghijklmnopqrstuvwxyz"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                    "0123456789";

    static const size_t S_LEN = 20;

    std::random_device rd;
    std::default_random_engine rng(rd());
    std::uniform_int_distribution<> dist(0,sizeof(alphabet)/sizeof(*alphabet)-2);

    std::string str;
    str.reserve(S_LEN);
    std::generate_n(std::back_inserter(str), S_LEN, [&]() { return alphabet[dist(rng)];});

    return str;
}



uint32_t process(const std::string &url)
{
    string random_url = get_random_str() + "." + url;
    auto domain_ptr = ldns_dname_new_frm_str(random_url.c_str());

    if (!domain_ptr)
        throw std::runtime_error { "ldns_dname_new_frm_str failure" };

    ldns_resolver * resolver_ptr { nullptr };

    if (ldns_resolver_new_frm_file(&resolver_ptr, nullptr) != LDNS_STATUS_OK)
        throw std::runtime_error { "ldns_resolver_new_frm_file failure failure" };

    auto packet = ldns_resolver_query(resolver_ptr,
                                      domain_ptr,
                                      LDNS_RR_TYPE_A,
                                      LDNS_RR_CLASS_IN,
                                      LDNS_RD);
    ldns_rdf_deep_free(domain_ptr);

    if (!packet)
        throw std::runtime_error { "ldns_resolver_query failure" };

    return ldns_pkt_querytime(packet);
}


int
main(int argc, char *argv[])
{
    std::signal(SIGINT,  signal_handler);
    std::signal(SIGTERM, signal_handler);

    vector<string> domains{"google.com", "facebook.com ", "youtube.com ",
                           "yahoo.com", "live.com", "wikipedia.org",
                           "baidu.com", "blogger.com", "msn.com", "qq.com"};

    ThreadPool pool(20);
    using respond = tuple<string, uint32_t, struct timeval>;
    Queue<respond> queue;

    std::unordered_map <string, int> m;

    thread producer([&] {
        shcedule(chrono::seconds(1), [&] {
            for (auto const &domain: domains) {
                pool.enqueue([&] {
                    uint32_t ms = process(domain);
                    struct timeval t;
                    queue.push(make_tuple(domain, ms, t));
                });
            }
        });
    });


    thread consumer([&queue](){
        shcedule(chrono::seconds(1), [&queue] {
            cout << "################################################" << endl;
           // auto current_size =  queue.size();
            auto current = queue.popAll();
       //     std::sort(current.begin(), current.end());
            //  for (int i = 0; i < current_size; i++) {
            for (auto const &response: current)
            //    auto response = queue.pop();
                cout << setw(20) << left << get<0>(response) <<
                     setw(20) << left << get<1>(response) <<
                     get<2>(response).tv_usec << endl;

        });
    });

    consumer.join();
    producer.join();

    // Get database access parameters from command line
    // Connect to the sample database.
    mysqlpp::Connection conn;
    if (conn.connect("TrackDNS", "127.0.0.1", "root", "password")) {
        // Retrieve a subset of the sample stock table set up by resetdb
        // and display it.
        mysqlpp::Query query = conn.query("select dns_name from stats");
        if (mysqlpp::StoreQueryResult res = query.store()) {
            cout << "We have:" << endl;
            mysqlpp::StoreQueryResult::const_iterator it;
            for (it = res.begin(); it != res.end(); ++it) {
                mysqlpp::Row row = *it;
                cout << '\t' << row[0] << endl;
            }
        }
        else {
            cerr << "Failed to get item list: " << query.error() << endl;
            return EXIT_FAILURE;
        }

        return EXIT_SUCCESS;
    }
    else {
        cerr << "DB connection failed: " << conn.error() << endl;
        return EXIT_FAILURE;
    }
}

