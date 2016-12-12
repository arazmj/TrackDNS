#include <iostream>
#include <string>
#include <thread>
#include <csignal>

#include <boost/program_options.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/bind.hpp>
#include <boost/thread/thread.hpp>


#include "ldns/ldns.h"
#include "mysql++/mysql++.h"

#include "Domain.h"
#include "Persistence.h"

//TODO add namespace for the whole program
using namespace boost::program_options;

template<class R, class P>
void shcedule(const std::chrono::duration<R, P> &duration, std::function<void()> func) {
    while (true) {
        func();
        std::this_thread::sleep_for(duration);
    }
}

std::vector<Domain *> sort_domain_ptrs(std::vector<Domain> &domains);

int main(int argc, const char *argv[]) {
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
            ("refresh,r", value<unsigned>()->default_value(1), "Frequency of database/display updates per second");

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

        bool stop_flag = false;
        boost::asio::io_service io_service;
        boost::thread_group pool;
        boost::asio::signal_set signals(io_service, SIGINT, SIGTERM);
        //boost::bind(&boost::asio::io_service::stop, &io_service)
        //signals.async_wait([&io_service, &stop_flag] { io_service.stop(); stop_flag = true; });

//        signals.async_wait(boost::bind(handler, boost::ref(signals), _1, _2));


        for (int i = 0; i < n_thread; i++) {
            pool.create_thread(
                    boost::bind(&boost::asio::io_service::run, &io_service)
            );
        }

        pool.create_thread([&]{
                shcedule(std::chrono::milliseconds(1000 / freq), [&] {
                for (auto &domain: domains) {
                    io_service.post([&domain] { domain.Update(); });
                }
            });
        });

        pool.create_thread([&]() {
            /* coalesce number of display/db updates so we do not blow the db with so many updates */
            shcedule(std::chrono::seconds(refresh_rate), [&] {
                std::cout << std::endl;
                Domain::ShowHeaders();
                std::vector<Domain *> sorted_domains = sort_domain_ptrs(domains);
                /* persist objects */
                for (auto const &domain: sorted_domains) {
                    std::cout << *domain << std::endl;
                    persistence.SaveDomain(*domain);
                }
            });
        });

        pool.join_all();

        /* shutdown gracefully */
        std::cout << "Persisting last result set..." << std::endl;
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
    /* we can not sort swap domain objects directly so we sort the vector of domain pointers */
    std::__1::vector<Domain *> sorted_domains;
    sorted_domains.resize(domains.size());
    transform(domains.begin(), domains.end(), sorted_domains.begin(),
                               [](Domain &d) { return &d; });

    sort(sorted_domains.begin(), sorted_domains.end(),
                          [](const Domain *d1, const Domain *d2)
                          { return *d1 < *d2; } );

    return sorted_domains;
}

