#include <iostream>
#include <string>
#include <thread>
#include <future>
#include <csignal>
#include <random>
#include <map>

#include "ldns/ldns.h"
#include "mysql++/mysql++.h"

#include "Queue.h"
#include "ThreadPool.h"
#include "Domain.h"


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
        std::this_thread::sleep_for(duration);
    }
}


int
main(int argc, char *argv[])
{
    std::signal(SIGINT,  signal_handler);
    std::signal(SIGTERM, signal_handler);

    //TODO can we do this inline without calling the copy constructor?
    std::vector<Domain> domains; /* {"google.com", "facebook.com", "youtube.com",
                           "yahoo.com", "live.com", "wikipedia.org",
                           "baidu.com", "blogger.com", "msn.com",
                           "qq.com"};*/
    domains.reserve(10);
    domains.emplace_back("google.com");
    domains.emplace_back("facebook.com");
    domains.emplace_back("youtube.com");
    domains.emplace_back("yahoo.com");
    domains.emplace_back("live.com");
    domains.emplace_back("wikipedia.org");
    domains.emplace_back("baidu.com");
    domains.emplace_back("blogger.com");
    domains.emplace_back("msn.com");
    domains.emplace_back("qq.com");

    //TODO make this an input parameter
    ThreadPool pool(20);

    //TODO make this as long as the frequency provided by user
    std::thread producer([&] {
        shcedule(std::chrono::seconds(1), [&] {
            for (auto &domain: domains) {
                pool.enqueue([&] { domain.Update();  });
            }
        });
    });


    //TODO consumer to call persistence and display object
    std::thread consumer([&domains](){
        shcedule(std::chrono::seconds(1), [&domains] {
            std::cout << "################################################" << std::endl;
            for (auto const &domain: domains)
                std::cout << domain << std::endl;

        });
    });

    std::this_thread::sleep_for(std::chrono::seconds(1));
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
            std::cout << "We have:" << std::endl;
            mysqlpp::StoreQueryResult::const_iterator it;
            for (it = res.begin(); it != res.end(); ++it) {
                mysqlpp::Row row = *it;
                std::cout << '\t' << row[0] << std::endl;
            }
        }
        else {
            std::cerr << "Failed to get item list: " << query.error() << std::endl;
            return EXIT_FAILURE;
        }

        return EXIT_SUCCESS;
    }
    else {
        std::cerr << "DB connection failed: " << conn.error() << std::endl;
        return EXIT_FAILURE;
    }
}

