#include <iostream>
#include <string>
#include <vector>
#include <random>
#include <chrono>
#include <fstream>

struct word_pair{
    std::string word;
    std::string next_word;
    size_t frequency;
};

std::vector<std::string> common_words = {
    "the", "be", "to", "of", "and", "a", "in", "that", "have", "I",
    "it", "for", "not", "on", "with", "he", "as", "you", "do", "at",
    "this", "but", "his", "by", "from", "they", "we", "say", "her", "she",
    "or", "an", "will", "my", "one", "all", "would", "there", "their", "what",
    "so", "up", "out", "if", "about", "who", "get", "which", "go", "me"
};

void to_lowercase(std::string& str){
    for (char& c : str){
        c = std::tolower(c);
    }
}

bool isValid(char ch){
    if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || (ch == ' ' || ch == '.' || ch == '!' || ch == '?' || ch == ':' || ch == ';')) return true;
    return false;
}

std::vector<std::string> string_to_words(std::string& text){
    std::vector<std::string> words;
    size_t n = text.length();
    to_lowercase(text);

    std::string word = "";
    for (size_t i = 0; i < n; i++){
        char ch = text[i];
        if (!isValid(ch)) continue;
        if (ch == ' '){
            if (word == "" || word == " ") continue;
            words.push_back(word);
            word = "";
        }
        else if (ch == '.' || ch == '!' || ch == '?' || ch == ':' || ch == ';'){
            if (word != "" && word != " "){
                words.push_back(word);
                word = "";
            }
            words.push_back("\\n");
        }
        else{
            word += ch;
        }
    }

    if (word != "" && word != " ") words.push_back(word);

    return words;
}

std::vector<word_pair> build_bigram_frequency_map(std::vector<std::string>& words){
    size_t n = words.size()-1;
    std::vector<word_pair> bigram_frequency_map;
    for (size_t i = 0; i < n; i++){
        if (words[i] == "\\n" || words[i+1] == "\\n") continue;
        bool isPresent = false;
        for (size_t j = 0; j < bigram_frequency_map.size(); j++){
            if (bigram_frequency_map[j].word == words[i] && bigram_frequency_map[j].next_word == words[i+1]){
                isPresent = true;
                bigram_frequency_map[j].frequency++;
                break;
            }
        }
        if (!isPresent){
            word_pair wp;
            wp.word = words[i];
            wp.next_word = words[i+1];
            wp.frequency = 1;
            bigram_frequency_map.push_back(wp);
        }
    }

    return bigram_frequency_map;
}

template <typename T>
T generate_random_number(T min, T max) {
    static std::mt19937_64 rng(std::chrono::steady_clock::now().time_since_epoch().count());
    if constexpr (std::is_same<T, bool>::value) {
        std::bernoulli_distribution dist((min || max) ? 0.5 : 0.0);
        return dist(rng);
    } else if constexpr (std::is_floating_point<T>::value) {
        std::uniform_real_distribution<T> dist(min, max);
        return dist(rng);
    } else if constexpr (std::is_integral<T>::value) {
        std::uniform_int_distribution<T> dist(min, max);
        return dist(rng);
    } else {
        throw std::runtime_error("Random generation not supported for this type!");
    }
}

std::string random_common_word(){
    int min = 0, max = 49;
    int random_index = generate_random_number(min, max);
    return common_words[random_index];
}

std::string most_frequent_word(std::vector<word_pair>& bigram_frequency_map){
    size_t n = bigram_frequency_map.size();
    std::string most_frequent_word = bigram_frequency_map[0].next_word;
    size_t frequency = 0;

    std::vector<std::string> words;
    std::vector<size_t> frequencies;

    for (size_t i = 0; i < n; i++){
        size_t m = words.size();
        bool word_found = false;
        bool next_word_found = false;
        for (size_t j = 0; j < m; j++){
            if (words[j] == bigram_frequency_map[i].word){
                word_found = true;
                frequencies[j] += bigram_frequency_map[i].frequency;
                if (frequencies[j] > frequency){
                    frequency = frequencies[j];
                    most_frequent_word = words[j];
                }
            }
            if (words[j] == bigram_frequency_map[i].next_word){
                next_word_found = true;
                frequencies[j] += bigram_frequency_map[i].frequency;
                if (frequencies[j] > frequency){
                    frequency = frequencies[j];
                    most_frequent_word = words[j];
                }
            }
        }
        if (!word_found){
            words.push_back(bigram_frequency_map[i].word);
            frequencies.push_back(bigram_frequency_map[i].frequency);
            m = frequencies.size() - 1;
            if (frequencies[m] > frequency){
                frequency = frequencies[m];
                most_frequent_word = words[m];
            }
        }
        if (!next_word_found){
            words.push_back(bigram_frequency_map[i].next_word);
            frequencies.push_back(bigram_frequency_map[i].frequency);
            m = frequencies.size() - 1;
            if (frequencies[m] > frequency){
                frequency = frequencies[m];
                most_frequent_word = words[m];
            }
        }
    }

    return most_frequent_word;
}

std::string predict_next_word(std::string word, std::vector<word_pair>& bigram_frequency_map){
    std::string predicted_word = "";
    size_t n = bigram_frequency_map.size();
    size_t highest_frequency = 0;
    to_lowercase(word);

    for (size_t i = 0; i < n; i++){
        if (bigram_frequency_map[i].word == word){
            if (bigram_frequency_map[i].frequency > highest_frequency){
                highest_frequency = bigram_frequency_map[i].frequency;
                predicted_word = bigram_frequency_map[i].next_word;
            }
        }
    }

    if (predicted_word == ""){
        int min = 1, max = 10;
        int decision = generate_random_number(min, max);

        if (decision < 10) predicted_word = random_common_word();
        else predicted_word = most_frequent_word(bigram_frequency_map);
    }

    return predicted_word;
}

bool read_file_to_string(const std::string& filename, std::string& output) {
    std::ifstream file(filename);
    if (!file.is_open()) return false;

    output.assign((std::istreambuf_iterator<char>(file)),
                   std::istreambuf_iterator<char>());
    return true;
}

bool model(std::string starting_word, size_t words_to_generate, std::string file_path_for_training){
    std::string text;
    if (read_file_to_string(file_path_for_training, text)){
        std::vector<std::string> words = string_to_words(text);
        std::vector<word_pair> bigram_frequency_map = build_bigram_frequency_map(words);
        std::string predicted_word = predict_next_word(starting_word, bigram_frequency_map);

        std::cout << starting_word;
        for (size_t i = 0; i < words_to_generate; i++){
            std::cout << " " << predicted_word;
            predicted_word = predict_next_word(predicted_word, bigram_frequency_map);
        }
        std::cout << "." << std::endl;

        return true;
    }
    else{
        return false;
    }
}


int main(){
    if (!model("you", 11, "training data/My Musical Life by Walter Damrosch(1000 lines).txt")) std::cout << "Something went wrong!" << std::endl;

    return 0;
}