#include <iostream>
#include <string>
#include <vector>
#include <curl/curl.h>

static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* data) {
    size_t newLength = size * nmemb;
    try {
        data->append((char*)contents, newLength);
    }
    catch (std::bad_alloc& e) {
        
        return 0;
    }
    return newLength;
}

std::string GetWikiPage(const std::string& pageName) {
    std::string readBuffer;
    CURL* curl = curl_easy_init();
    if (curl) {
        std::string url = "https://en.wikipedia.org/wiki/" + pageName;
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        curl_easy_perform(curl);
        curl_easy_cleanup(curl);
    }
    return readBuffer;
}

std::vector<std::string> ExtractLinks(const std::string& content) {
    std::vector<std::string> links;
    std::size_t start_pos = 0;
    while ((start_pos = content.find("<a href=\"/wiki/", start_pos)) != std::string::npos) {
        start_pos += 15; 
        std::size_t end_pos = content.find("\"", start_pos);
        if (end_pos != std::string::npos) {
            std::string link = content.substr(start_pos, end_pos - start_pos);
            if (link.find(":") == std::string::npos) { 
                links.push_back(link);
            }
            start_pos = end_pos;
        }
    }
    return links;
}

bool FindWikiLadder(const std::string& startPage, const std::string& endPage, std::vector<std::string>& ladder) {
    std::vector<std::string> visited;
    std::vector<std::vector<std::string>> queue;
    queue.push_back({ startPage });

    while (!queue.empty()) {
        std::vector<std::string> currentPath = queue.front();
        queue.erase(queue.begin());
        std::string currentPage = currentPath.back();
        std::string content = GetWikiPage(currentPage);
        std::cout << "Retrieved URL: https://en.wikipedia.org/wiki/" << currentPage << std::endl;
        if (currentPage == endPage) {
            ladder = currentPath;
            return true;
        }

        visited.push_back(currentPage);
        std::vector<std::string> links = ExtractLinks(content);
        for (const std::string& link : links) {
            if (std::find(visited.begin(), visited.end(), link) == visited.end()) {
                std::vector<std::string> newPath = currentPath;
                newPath.push_back(link);
                queue.push_back(newPath);


            }
        }
    }
    return false;
}

int main() {
    std::string startPage = "Tribe";
    std::string endPage = "Kinship";
    std::vector<std::string> ladder;

    if (FindWikiLadder(startPage, endPage, ladder)) {
        std::cout << "Wiki ladder found!\n";
        for (const std::string& page : ladder) {
            std::cout << page << " -> ";
        }
        std::cout << "Goal reached: " << endPage << std::endl;
    }
    else {
        std::cout << "No ladder found between " << startPage << " and " << endPage << std::endl;
    }

    return 0;
}
