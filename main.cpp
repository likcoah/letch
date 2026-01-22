#include <iostream>
#include <filesystem>
#include <fstream>
#include <string>
#include <regex>
#include <sstream>
#include <cstdlib>
#include <vector>
#include <unordered_map>


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
	std::string distro_name;
	std::vector<std::string> distro_logo;
	std::unordered_map<std::string, std::string> system_data;
	std::vector<std::string> system_data_iterator;
};


namespace InitDistroData
{
	std::string distroName()
	{
		if (std::ifstream os_release("/etc/os-release"); os_release) {
			static const std::regex pattern(R"(^ID="?([^"\n]+)\"?)");
			for (std::string line; std::getline(os_release, line); ) {
				if (std::smatch match; std::regex_match(line, match, pattern)) return match.str(1);
			}
			return "linux";
		} else return "linux";
	}


	std::vector<std::string> distroLogo(const std::filesystem::path& logo_path)
	{
		std::vector<std::string> distro_logo;
		if (std::ifstream ascii_logo(logo_path); ascii_logo) {
			for (std::string line; std::getline(ascii_logo, line); ) {
				distro_logo.push_back(line);
			}
		}
		return distro_logo;
	}


	FetchData fetchData(const char* distro_name = nullptr)
	{
		FetchData fetch_data;
		const std::filesystem::path& source_dir = SourceDir::getSourceDir();

		{
			if (distro_name) fetch_data.distro_name = distro_name;
			else fetch_data.distro_name = distroName();
		}

		{
			std::filesystem::path logo_path = source_dir / "logos";
			if (std::filesystem::exists(logo_path / fetch_data.distro_name)) fetch_data.distro_logo = distroLogo(logo_path / fetch_data.distro_name);
			else fetch_data.distro_logo = distroLogo(logo_path / "linux");
		}

		{
			const char* raw_username = std::getenv("USER");
			const char* raw_logname = std::getenv("LOGNAME");
			if (raw_username) fetch_data.system_data["username"] = raw_username;
			else if (raw_logname) fetch_data.system_data["username"] = raw_logname;
			else fetch_data.system_data["username"] = "root";

			std::string hostname;
			if (std::ifstream hostname_read("/etc/hostname");
					hostname_read && std::getline(hostname_read, hostname) &&
					!hostname.empty()) fetch_data.system_data["hostname"] = hostname;
			else fetch_data.system_data["hostname"] = "localhost";
		}

		return fetch_data;
	}
};


void render(const FetchData& fetch_data)
{
	constexpr std::string_view esc = "\033", line_break = "\n", space = " ", tab = "    ", reset = "[0m",
			  border_color = "[90m", border_vertical = "│", border_horizontal = "─", border_right_bottom_corner = "┘",
			  border_vertical_and_right = "├", border_horizontal_and_up = "┴";
			  
	int length = 17;
	{
		int index = 0;
		for (int i = 0; i < fetch_data.distro_logo.size(); i++) {
			if (fetch_data.distro_logo[i] > fetch_data.distro_logo[index]) index = i;
		}
		const size_t pos = fetch_data.distro_logo[index].find('m');
		const std::string_view content(fetch_data.distro_logo[index].c_str() + pos + 1);
		length += content.length() - 1;
	}

	std::stringstream output;
	const std::string spacing_buffer(length, ' ');
	
	output << spacing_buffer << esc << border_color << border_vertical << esc << reset <<
		space << fetch_data.system_data.at("username") << "@" << fetch_data.system_data.at("hostname") << line_break;

	int line_index = 0;
	for (const std::string& line : fetch_data.distro_logo) {
		output << tab << tab << esc << line << esc << reset << tab << tab;

		if (line_index == 0) {
			output << esc << border_color << border_vertical_and_right;
			for (int index = 15; index > 0; index--) output << border_horizontal;
		} else {
			output << esc << border_color << border_vertical;
		}

		output << esc << reset << line_break;
		line_index++;
	}

	output << spacing_buffer << esc << border_color << border_vertical << esc << reset <<
		line_break << esc << border_color;
	for (int index = 0; index < length; index++) output << border_horizontal;
	output << border_right_bottom_corner << esc << reset << line_break;

	std::cout << output.str();
}


int main(int argc, char* argv[])
{
	SourceDir::setSourceDir(argv[0]);

	FetchData fetch_data;
	if (argc > 1) fetch_data = InitDistroData::fetchData(argv[1]);
	else fetch_data = InitDistroData::fetchData();

	render(fetch_data);
	return 0;
}

