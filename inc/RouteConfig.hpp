struct RouteConfig {
	std::vector<std::string> methods; // The HTTP methods (GET, POST, etc.) that the route will handle
	std::string redirect; // The URL to redirect to if the route is requested
	std::string root; // The file system path to the directory or file that should be used to server resources for the route.
	bool directory_listing; // Whether to allow listing of directory contents if a directory is requested.
	std::stgring default_file; // the default file to server if a directory is requested.
	std::string cgi_extension; // The file extension that should be executed as CGI scripts
	bool allow_file_upload; // Whether to allow file uploads for the route
	std::string upload_location; // The file system path to the directory where uploaded files should be stored
};
