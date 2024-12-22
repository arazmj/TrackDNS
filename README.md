# TrackDNS

TrackDNS is a performance monitoring tool for top DNS servers. It measures the response times of various DNS servers and stores the results in a database for further analysis. The tool supports multithreading for efficient processing and periodic updates for database persistence and display.

# Features
Measure DNS server performance for popular domains.
Multithreaded processing using Boost Asio.
Configurable query frequency and update intervals.
Results are persisted in a MySQL database.
Graceful shutdown with proper resource cleanup.
Sorting and displaying domain performance metrics.
Requirements

# Libraries
Boost: Used for asynchronous I/O, threading, and signal handling.
LDNS: For DNS querying.
MySQL++: For interacting with the MySQL database.
System
A C++ compiler supporting C++11 or later.
A MySQL database server.
Installation

Clone the repository:
```
git clone https://github.com/yourusername/TrackDNS.git
cd TrackDNS
Install dependencies:
On Ubuntu:
sudo apt-get install libboost-all-dev libldns-dev libmysql++-dev
On macOS (using Homebrew):
brew install boost ldns mysql++
```

Compile the project:
```
make
Usage
```

Run the program with the following options:
```
./TrackDNS [options]
Command-Line Options
Option	Description	Default
--help	Show help message.
--frequency, -f	Number of queries per second.	20
--database, -d	Name of the database.	TrackDNS
--host, -h	Hostname of the database server.	127.0.0.1
--username, -u	Database username.	root
--password, -p	Database password.	password
--threads, -t	Number of threads in the thread pool.	20
--refresh, -r	Frequency of database/display updates per second.	1
```

# Example
Run TrackDNS with a query frequency of 50 queries per second:
```
./TrackDNS -f 50 -d TrackDNS -u admin -p secretpassword
```
# How It Works

Domain Initialization: A list of popular domains is initialized.
Database Connection: Connect to the MySQL database.
Asynchronous Processing: Use Boost Asio for scheduling periodic tasks.
DNS Queries: Send DNS queries and update domain metrics.
Results Persistence: Save results to the database periodically.
Sorting and Display: Sort domains by performance and display metrics.
Graceful Shutdown

The application handles termination signals (SIGINT, SIGTERM) to stop processing and persist the final results before exiting.

# Contributing

Contributions are welcome! Please fork the repository, make your changes, and submit a pull request.

# License

This project is licensed under the MIT License.

# Acknowledgments

Boost C++ Libraries
LDNS Library
MySQL++
Feel free to reach out if you encounter any issues or have feature suggestions!

# TrackDNS
Check top DNS servers performance

```bash
TrackDNS, measures top DNS servers performance:
  --help                            Help
  -f [ --frequency ] arg (=20)      Frequency, number of queries per second
  -d [ --database ] arg (=TrackDNS) Database name
  -h [ --host ] arg (=127.0.0.1)    Database host name
  -u [ --username ] arg (=root)     Database username
  -p [ --password ] arg (=password) Database password
  -t [ --threads ] arg (=20)        Number of threads for the thread pool
  -r [ --refresh ] arg (=1)         Frequency of database and display update 
                                    per second                                  
  example: ./TrackDNS -f 10 -d TrackDNS -h 127.0.0.1 -u root -p password
```
                                    

| DNS           | Current | Last TS | First TS | Count | Average | Deviation | 
|---------------|---------|---------|----------|-------|---------|-----------| 
| google.com    | 45      | 48008   | 153211   | 282   | 577.197 | 2.34871   | 
| facebook.com  | 32      | 47931   | 153210   | 283   | 521.663 | 1.73256   | 
| youtube.com   | 32      | 47931   | 153210   | 283   | 569.042 | 2.29758   | 
| yahoo.com     | 44      | 47931   | 153212   | 283   | 622.549 | 1.53519   | 
| live.com      | 56      | 47966   | 153212   | 284   | 646.291 | 2.21074   | 
| wikipedia.org | 44      | 47941   | 153300   | 282   | 879.336 | 4.8303    | 
| baidu.com     | 342     | 547861  | 154117   | 268   | 3631.9  | 40.5146   | 
| blogger.com   | 31      | 48105   | 153211   | 283   | 592.794 | 2.47657   | 
| msn.com       | 48      | 48188   | 153210   | 284   | 681.972 | 2.55711   | 
| qq.com        | 320     | 546048  | 153276   | 260   | 6742.68 | 54.298    | 

