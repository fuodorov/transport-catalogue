# Transport Catalogue

## Build and launch

To build our application and create a Docker image, it will be enough to run the following command:

`docker build -t transport_catalogue .`

To launch the application, use the command:

`docker run -it transport_catalogue`

#### Two-stage in the transport directory

* `make_base` - creation of a transport directory database based on `base_requests` queries and its serialization into a file.

* `process_requests` - deserializing the database from a file and using it to respond to `stat_requests` requests.
