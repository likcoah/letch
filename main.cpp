#include <iostream>
#include <filesystem>
#include <fstream>
#include <string>
#include <regex>


class SourceDir
{
	private:
		static std::filesystem::path& sourceDir()
		{
			static std::filesystem::path source_dir;
			return source_dir;
		}

	public:
		static void setSourceDir(const char* binary_file)
		{
			sourceDir() = std::filesystem::weakly_canonical(std::filesystem::absolute(binary_file).parent_path());
		}
		
		const static std::filesystem::path& getSourceDir()
		{
			return sourceDir();
		}
};


struct FetchData
{
	std::string distro_name = "linux";
	std::vector<std::string> distro_logo;

	void initDistroName()
	{
		if (std::ifstream os_release("/etc/os-release"); os_release) {
			const std::regex pattern(R"(^ID="?([^"\n]+)\"?)");
			for (std::string line; std::getline(os_release, line); ) {
				if (std::smatch match; std::regex_match(line, match, pattern)) {
					distro_name = match[1];
					return;
				} else continue;
			}
		}
	}

	void initDistroLogo()
	{
		const std::filesystem::path& source_dir = SourceDir::getSourceDir();
		std::filesystem::path logo_path;
		
		if (std::filesystem::exists(source_dir / "logos" / distro_name)) logo_path = source_dir / "logos" / distro_name;
		else logo_path = source_dir / "logos" / "linux";

		if (std::ifstream logo_art(logo_path); logo_art) {
			for (std::string line; std::getline(logo_art, line); ) {
				distro_logo.push_back(line);
			}
		}
	}
};


void render(const FetchData& fetch_data)
{
	constexpr std::string_view esc = "\033";
	for (const std::string& line : fetch_data.distro_logo) {
		std::cout << esc << line << esc << "[0m" << std::endl;
	}
}


int main(int argc, char* argv[])
{
	SourceDir::setSourceDir(argv[0]);
	FetchData fetch_data;
	if (argc != 1) fetch_data.distro_name = argv[1];
	else fetch_data.initDistroName();
	fetch_data.initDistroLogo();
	render(fetch_data);
	return 0;
}

