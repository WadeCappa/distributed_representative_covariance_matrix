#include "normalizer.h"
#include "data_saver.h"

#include <utility>
#include <functional>
#include <doctest/doctest.h>

static std::vector<std::vector<double>> DATA = {
    {4,17,20},
    {5,7,31},
    {2.63212,5.12566,763}
};

static std::pair<std::vector<double>, double> VECTOR_WITH_LENGTH = std::make_pair(
    std::vector<double>{3.0, 3.0, 3.0, 3.0},
    6.0
);

static std::string rowToString(const std::vector<double> &row) {
    std::ostringstream outputStream;
    for (const auto & v : row) {
        outputStream << v << ",";
    }
    std::string output = outputStream.str();
    output.pop_back();
    return output;
}

static std::string matrixToString(const std::vector<std::vector<double>> &data) {
    std::string output = "";
    for (const auto & row : data) {
        output += rowToString(row);
        output.append("\n");
    }

    return output;
}

static void validateNormalizedVector(const std::vector<double> &data) {
    for (const auto & e : data) {
        CHECK(e <= 1.0);
    }

    double length = Normalizer::vectorLength(data);
    CHECK(length > 0.9);
    CHECK(length < 1.1);
}

static std::vector<std::vector<double>> loadData(DataLoader &loader) {
    std::vector<std::vector<double>> data;
    std::vector<double> element;
    while (loader.getNext(element)) {
        data.push_back(element);
    }

    return data;
}

static void DEBUG_printData(const std::vector<std::vector<double>> &data) {
    for (const auto & d : data) {
        for (const auto & v : d) {
            std::cout << v << ", ";
        }
        std::cout << std::endl;
    }
}

TEST_CASE("Testing loading data") {
    std::string dataAsString = matrixToString(DATA);
    std::istringstream inputStream(dataAsString);
    AsciiDataLoader loader(inputStream);

    auto data = loadData(loader);

    CHECK(data == DATA);
}

TEST_CASE("Testing length calculation") {
    double length = Normalizer::vectorLength(VECTOR_WITH_LENGTH.first);
    CHECK(length == VECTOR_WITH_LENGTH.second);
}

TEST_CASE("Testing vector normalization") {
    for (const auto & e : DATA) {
        std::vector<double> copy = e;
        Normalizer::normalize(copy);

        validateNormalizedVector(copy);
    }
}

TEST_CASE("Testing the normalized data loader") {
    std::istringstream inputStream(matrixToString(DATA));
    AsciiDataLoader dataLoader(inputStream);
    Normalizer normalizer(dataLoader);
    std::vector<double> element;
    while (normalizer.getNext(element)) {
        validateNormalizedVector(element);
    }
}

TEST_CASE("Testing saving data") {
    std::string dataAsString = matrixToString(DATA);
    std::ostringstream stringStream;

    std::istringstream inputStream(dataAsString);
    AsciiDataLoader dataLoader(inputStream);
    AsciiDataSaver saver(dataLoader);
    stringStream << saver;
    CHECK(stringStream.str() == dataAsString);
}

TEST_CASE("Testing loading and saving binary data") {
    std::string dataAsString = matrixToString(DATA);
    std::istringstream asciiStream(dataAsString);

    AsciiDataLoader asciiDataLoader(asciiStream);
    BinaryDataSaver saver(asciiDataLoader);

    std::ostringstream outputStream;
    outputStream << saver;

    std::istringstream binaryStream(outputStream.str());
    BinaryDataLoader binaryDataLoader(binaryStream);

    std::vector<std::vector<double>> data = loadData(binaryDataLoader);

    CHECK(data == DATA);
}