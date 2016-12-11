#include <iostream>
#include <string>
#include <thread>
#include <csignal>

#include <boost/program_options.hpp>

#include "ldns/ldns.h"
#include "mysql++/mysql++.h"

#include "ThreadPool.h"
#include "Domain.h"
#include "Persistence.h"

//TODO add namespace for the whole program
using namespace boost::program_options;

volatile bool stop_flag = false;
void signal_handler(int signal) { stop_flag = true; }

template<class R, class P>
void shcedule(const std::chrono::duration<R, P> &duration, std::function<void()> func) {
    while (!stop_flag) {
        func();
        std::this_thread::sleep_for(duration);
    }
}

std::vector<Domain *> sort_domain_ptrs(std::vector<Domain> &domains);

int main(int argc, const char *argv[]) {
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    options_description description("\nTrackDNS, measures top DNS servers performance");
    variables_map vm;

    description.add_options()
            ("help", "Help")
            ("frequency,f", value<unsigned>()->default_value(20), "Frequency, number of queries per second")
            ("database,d", value<std::string>()->default_value("TrackDNS"), "Database name")
            ("host,h", value<std::string>()->default_value("127.0.0.1"), "Database host name")
            ("username,u", value<std::string>()->default_value("root"), "Database username")
            ("password,p", value<std::string>()->default_value("password"), "Database password")
            ("threads,t", value<unsigned>()->default_value(20), "Number of threads for the thread pool")
            ("refresh,r", value<unsigned>()->default_value(1), "Frequency of database and display update per second");

    int freq = 0, refresh_rate = 0, n_thread = 0;
    std::string db_name, db_host, db_user, db_password;

    /* parse command line arguments */
    try {
        store(parse_command_line(argc, argv, description), vm);

        if (vm.count("help")) {
            std::cout << description << std::endl;
            return EXIT_SUCCESS;
        }

        freq = vm["frequency"].as<unsigned>();
        db_name = vm["database"].as<std::string>();
        db_host = vm["host"].as<std::string>();
        db_user = vm["username"].as<std::string>();
        db_password = vm["password"].as<std::string>();
        n_thread = vm["threads"].as<unsigned>();
        refresh_rate = vm["refresh"].as<unsigned>();
    }
    catch (const std::exception &e) {
        std::cerr << e.what() << description << std::endl;
        return EXIT_FAILURE;
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

    try {
        Persistence persistence(db_name, db_host, db_user, db_password);

        for (auto &domain: domains) {
            persistence.LoadDomain(domain);
        }

        ThreadPool pool((size_t) n_thread);

        std::thread producer([&] {
            shcedule(std::chrono::milliseconds(1000 / freq), [&] {
                for (auto &domain: domains) {
                    pool.enqueue([&domain] { domain.Update(); });
                }
            });
        });

        std::thread consumer([&]() {
            /* coalesce number of display/db updates so we do not blow the db with so many updates */
            shcedule(std::chrono::seconds(refresh_rate), [&] {
                std::cout << std::endl;
                Domain::ShowHeaders();
                /* we can not sort swap domain objects directly so we sort the vector of domain pointers */
                std::vector<Domain *> sorted_domains = sort_domain_ptrs(domains);
                /* persist objects */
                for (auto const &domain: sorted_domains) {
                    std::cout << *domain << std::endl;
                    persistence.SaveDomain(*domain);
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

    } catch (const mysqlpp::ConnectionFailed &e) {
        std::cerr << "mysqlpp::ConnectionFailed " << e.what() << std::endl;
        std::cerr << db_user << "@" << db_host << " Password: " << db_password << std::endl;
    } catch (const mysqlpp::BadQuery &e) {
        std::cerr << "mysqlpp::BadQuery " << e.what() << std::endl;
        std::cerr << db_user << "@" << db_host << " Password: " << db_password << std::endl;
    }

    return EXIT_SUCCESS;
}

std::vector<Domain *> sort_domain_ptrs(std::vector<Domain> &domains) {
    std::__1::vector<Domain *> sorted_domains;
    sorted_domains.resize(domains.size());
    transform(domains.begin(), domains.end(), sorted_domains.begin(),
                               [](Domain &d) { return &d; });

    sort(sorted_domains.begin(), sorted_domains.end(),
                          [](const Domain *d1, const Domain *d2)
                          { return *d1 < *d2; } );

    return sorted_domains;
}

