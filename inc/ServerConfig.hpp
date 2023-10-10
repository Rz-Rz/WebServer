struct ServerConfig {
	std::string host; // IP address of the server
	int port;
	std::string server_name; // The name of the server
	std::string default_error_page; // Path to the files or messages to be used as responses for specific HTTP error statuses, like 404 Not Found or 500 Internal error.
	int max_client_body_size; // The maximum size of of the HTTP reauest body the server will accept, often used to limit the size of the file uploads.
	std::map<std::string, RouteConfig> routes; // <path, RouteConfig>
};
