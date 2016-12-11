# TrackDNS
Check top DNS performance

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

