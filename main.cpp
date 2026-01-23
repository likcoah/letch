#include <iostream>
#include <filesystem>
#include <fstream>
#include <string>
#include <regex>
#include <sstream>
#include <cstdlib>
#include <vector>
#include <unordered_map>
#include <optional>


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


class FetchData
{
	public:
		std::string distro_name;
		std::vector<std::string> distro_logo;
		std::unordered_map<std::string, std::string> system_data;
		std::vector<std::string> system_data_iterator;

		
		FetchData(const std::optional<std::string_view> argument_distro_name = std::nullopt)
		{
			if (argument_distro_name) distro_name = *argument_distro_name;
			else initDistroName();

			initDistroLogo();
			initUsername();
			initHostname();
		}
	
	private:
		void initDistroName()
		{
			if (std::ifstream os_release("/etc/os-release"); os_release) {
				static const std::regex pattern(R"(^ID="?([^"\n]+)\"?)");
				for (std::string line; std::getline(os_release, line); ) {
					if (std::smatch match; std::regex_match(line, match, pattern)) {
						distro_name = match.str(1);
						return;
					}
				}
				distro_name = "linux";
			} else distro_name = "linux";
		}


		void initDistroLogo()
		{
			std::vector<std::string> result;
			const std::filesystem::path& logos_path = SourceDir::getSourceDir() / "logos";

			if (std::ifstream logo(logos_path / distro_name); logo) {
				for (std::string line; std::getline(logo, line); ) result.push_back(line);
			} else if (std::ifstream logo(logos_path / "linux"); logo) {
				for (std::string line; std::getline(logo, line); ) result.push_back(line);
			}
			distro_logo = result;
		}


		void initUsername()
		{
			const char* raw_username = std::getenv("USER");
			const char* raw_logname = std::getenv("LOGNAME");
			if (raw_username) system_data["username"] = raw_username;
			else if (raw_logname) system_data["username"] = raw_logname;
			else system_data["username"] = "root";
		}


		void initHostname()
		{
			std::string hostname;
			if (std::ifstream hostname_read("/etc/hostname");
					hostname_read && std::getline(hostname_read, hostname) &&
					!hostname.empty()) system_data["hostname"] = hostname;
			else system_data["hostname"] = "localhost";
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

	std::optional<std::string_view> arg;
	if (argc > 1) arg = argv[1];
	FetchData fetch_data(arg);

	render(fetch_data);
	return 0;
}

