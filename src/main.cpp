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
#include "Persistence.h"


bool stop_flag = false;

void signal_handler(int signal, std::function<void()> shutdown)
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
    int freq = 0;
    std::string db_name, db_host, db_user, db_password;

    if (argc < 6) {
        std::cout << "TrackDNS [frequency] [DB name] [DB Host] [DB User] [DB Password]" << std::endl;
        std::cout << "Example: ./TrackDNS 10 TrackDNS 127.0.0.1 root password" << std::endl;
        return EXIT_FAILURE;
    } else {
        freq = std::stoi(argv[1]);
        db_name = std::string(argv[2]);
        db_host = std::string(argv[3]);
        db_user = std::string(argv[4]);
        db_password = std::string(argv[5]);
    }


    std::vector<Domain> domains;

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

    Persistence persistence(db_name, db_host, db_user, db_password);

    for (auto &domain: domains) {
        persistence.LoadDomain(domain);
    }

    //TODO make this an input parameter
    ThreadPool pool(20);

    //TODO make this as long as the frequency provided by user
    std::thread producer([&] {
        shcedule(std::chrono::milliseconds(1000 / freq), [&] {
            for (auto &domain: domains) {
                pool.enqueue([&] { domain.Update();  });
            }
        });
    });


    //TODO consumer to call persistence and display object
    std::thread consumer([&](){
        shcedule(std::chrono::seconds(1), [&] {
            std::cout << std::endl;
            Domain::ShowHeaders();
            std::copy(domains.begin(), domains.end(), std::ostream_iterator<Domain>(std::cout, "\n"));
            for (auto const &domain: domains) {
                persistence.SaveDomain(domain);
            }
        });
    });

    std::this_thread::sleep_for(std::chrono::seconds(1));
    consumer.join();
    producer.join();

    /* shutdown gracefully */
    for (auto const &domain: domains) {
        persistence.SaveDomain(domain);
    }

    return EXIT_SUCCESS;
}

