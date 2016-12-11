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

template <class R, class P>
void shcedule(const std::chrono::duration<R, P>& duration, std::function<void()> func)
{
    while (!stop_flag) {
        func();
        std::this_thread::sleep_for(duration);
    }
}

int main(int argc, char *argv[])
{
    std::signal(SIGINT,  signal_handler);
    std::signal(SIGTERM, signal_handler);

    options_description description("TrackDNS, measures top DNS servers performance");
    variables_map vm;

    description.add_options()
            ("help", "Help")
            ("frequency,f", value<unsigned>()->default_value(20), "Frequency, number of queries per second")
            ("database,d", value<std::string>()->default_value("TrackDNS"), "Database name")
            ("host,h", value<std::string>()->default_value("127.0.0.1"), "Database host name")
            ("username,u", value<std::string>()->default_value("root"), "Database username")
            ("password,p", value<std::string>()->default_value("password"), "Database password");

    int freq = 0;
    std::string db_name, db_host, db_user, db_password;

    try
    {
        store(parse_command_line(argc, argv, description), vm);

        if (vm.count("help"))
        {
            std::cout << description << std::endl;
            return EXIT_SUCCESS;
        }

        freq = vm["frequency"].as<unsigned>();
        db_name = vm["database"].as<std::string>();
        db_host = vm["host"].as<std::string>();
        db_user = vm["username"].as<std::string>();
        db_password = vm["password"].as<std::string>();
    }
    catch (const std::exception &e)
    {
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

        //TODO make this an input parameter
        ThreadPool pool(20);

        std::thread producer([&] {
            shcedule(std::chrono::milliseconds(1000 / freq), [&] {
                for (auto &domain: domains) {
                    pool.enqueue([&domain] { domain.Update(); });
                }
            });
        });

        std::thread consumer([&]() {
            /* coalesce number of display/db updates so we do not blow the db with so many updates */
            //TODO make update time admin input
            shcedule(std::chrono::seconds(1), [&] {
                //TODO it would be nice to sort it by latency
                std::cout << std::endl;
                Domain::ShowHeaders();
                std::copy(domains.begin(), domains.end(), std::ostream_iterator<Domain>(std::cout, "\n"));

                /* persist objects */
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

    } catch (mysqlpp::ConnectionFailed e) {
        std::cerr << e.what()  << std::endl;
        std::cerr << db_user << "@" << db_host << " Password: " << db_password << std::endl;
    }

    return EXIT_SUCCESS;
}

