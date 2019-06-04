#ifndef __FILETYPE__
#define __FILETYPE__

#include <string>
#include <cstring>

struct ctypes {
	std::string extension, c_type;
};

struct ctypes content_types[] = {
	// This one must be at index 0 and 1, since we keep a cache of
	// file <-> content-type with an index to this table
	{".data", "application/data"},
	{".html", "text/html"},
	{".apk", "application/vnd.android.package-archive"},
	{".avi", "video/x-msvideo"},
	{".bmp", "image/bmp"},
	{".bib", "text/x-bibtex"},
	{".c", "text/x-csrc"},
	{".cc", "text/x-c++src"},
	{".cpp", "text/x-c++src"},
	{".cxx", "text/x-c++src"},
	{".css", "text/css"},
	{".dtd", "text/x-dtd"},
	{".dvi", "application/x-dvi"},
	{".fig", "image/x-xfig"},
	{".flv", "application/flash-video"},
	{".gif", "image/gif"},
	{".gz", "application/gzip"},
	{".h", "text/x-chdr"},
	{".hh", "text/x-chdr"},
	{".htm", "text/html"},
	{".ico", "image/x-ico"},
	{".iso", "application/x-cd-image"},
	{".java", "text/x-java"},
	{".jpg", "image/jpg"},
	{".js", "application/x-javascript"},
	{".mp3", "audio/mpeg"},
	{".mpeg", "video/mpeg"},
	{".mpg", "video/mpeg"},
	{".ogg", "application/ogg"},
	{".pac", "application/x-ns-proxy-autoconfig"},
	{".pdf", "application/pdf"},
	{".pls", "audio/x-scpls"},
	{".png", "image/png"},
	{".ps", "application/postscript"},
	{".ps.gz", "application/x-gzpostscript"},
	{".rar", "application/x-rar-compressed"},
	{".rdf", "text/rdf"},
	{".rss", "text/rss"},
	{".sgm", "text/sgml"},
	{".sgml", "text/sgml"},
	{".svg", "image/svg+xml"},
	{".tar", "application/x-tar"},
	{".tar.Z", "application/x-tarz"},
	{".tgz", "application/gzip"},
	{".tiff", "image/tiff"},
	{".txt", "text/plain"},
	{".wav", "audio/x-wav"},
	{".wmv", "video/x-ms-wm"},
	{".xbm", "image/x-xbitmap"},
	{".x509", "application/x-x509-ca-cert"},
	{".xml", "text/xml"},
	{".zip", "application/zip"},
	{".zoo", "application/x-zoo"},
	{"", ""}
};

int find_ctype(const string &p)
{
	int i = 0;
	for (i = 0; !content_types[i].extension.empty(); ++i) {
		if (!strcasecmp(p.c_str(),
        	       content_types[i].extension.c_str())){
			break;
		}
	}
	if (content_types[i].c_type.empty())
		i = 0;
	return i;
}

#endif //__FILETYPE__