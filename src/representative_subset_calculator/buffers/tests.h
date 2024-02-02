#include <vector>
#include <cstdlib>
#include "bufferBuilder.h"

std::vector<std::pair<size_t, double>> getMockSolution() {
    std::vector<std::pair<size_t, double>> mockSolution;
    mockSolution.push_back(std::make_pair(0, 10.0));
    mockSolution.push_back(std::make_pair(DATA.size() - 1, 5.0));
    return mockSolution;
}

TEST_CASE("Testing the get total send data method") {
    auto mockSolution = getMockSolution();
    std::vector<double> sendBuffer;
    unsigned int totalSendData = BufferBuilder::buildSendBuffer(data, mockSolution, sendBuffer);
    CHECK(totalSendData == (data.totalColumns() + 1) * mockSolution.size() + 1);
    CHECK(sendBuffer.size() == totalSendData);
}

TEST_CASE("Test building send buffer") {
    auto mockSolution = getMockSolution();
    std::vector<double> sendBuffer;
    unsigned int totalSendData = BufferBuilder::buildSendBuffer(data, mockSolution, sendBuffer);

    CHECK(sendBuffer.size() == totalSendData);
    for (size_t i = 0; i < mockSolution.size(); i++) {
        size_t sentRow = static_cast<size_t>(sendBuffer[i * (data.totalColumns() + 1) + 1]);
        CHECK(sentRow == mockSolution[i].first);
    }
}

TEST_CASE("Test to binary and back to matrix") {
    auto mockSolution = getMockSolution();
    std::vector<double> sendBuffer;
    unsigned int totalSendData = BufferBuilder::buildSendBuffer(data, mockSolution, sendBuffer);
    
    std::vector<int> displacements;
    displacements.push_back(0);

    BufferLoader bufferLoader(sendBuffer, data.totalColumns(), displacements);
    std::vector<std::pair<size_t, std::vector<double>>> newData = *bufferLoader.returnNewData().get();
    CHECK(newData.size() == mockSolution.size());

    for (size_t i = 0; i < mockSolution.size(); i++) {
        CHECK(newData[i].first == mockSolution[i].first);
        CHECK(newData[i].second == data.getRow(mockSolution[i].first));
    }

    // std::vector<std::pair<double, std::vector<int>>> localSolutions = *bufferLoader.returnLocalSolutions().get();
    // CHECK(localSolutions.size() == 1);

//     double totalScore = 0;
//     for (size_t i = 0; i < mockSolution.size(); i++) {
//         totalScore += mockSolution[i].second;
//         CHECK(mockSolution[i].first == localSolutions.back().second[i]);
//     }

//     CHECK(totalScore == localSolutions.back().first);
}

TEST_CASE("Mock MPI test sending and receiving a buffer") {
    auto mockSolutionRank0 = getMockSolution();
    auto mockSolutionRank1 = getMockSolution();

    std::vector<double> sendBufferRank0;
    std::vector<double> sendBufferRank1;
    unsigned int totalSendDataRank0 = BufferBuilder::buildSendBuffer(data, mockSolutionRank0, sendBufferRank0);
    unsigned int totalSendDataRank1 = BufferBuilder::buildSendBuffer(data, mockSolutionRank1, sendBufferRank1);

    std::vector<int> displacements;
    displacements.push_back(0);
    displacements.push_back(totalSendDataRank0);

    for (const auto & d : sendBufferRank1) {
        sendBufferRank0.push_back(d);
    }

    BufferLoader bufferLoader(sendBufferRank0, data.totalColumns(), displacements);
    std::vector<std::pair<size_t, std::vector<double>>> newData = *bufferLoader.returnNewData().get();
    CHECK(newData.size() == mockSolutionRank0.size() + mockSolutionRank1.size());

    for (size_t i = 0; i < mockSolutionRank0.size(); i++) {
        CHECK(newData[i].first == mockSolutionRank0[i].first);
        CHECK(newData[i].second == data.getRow(mockSolutionRank0[i].first));
    }

    for (size_t i = 0; i < mockSolutionRank1.size(); i++) {
        CHECK(newData[i + mockSolutionRank0.size()].first == mockSolutionRank1[i].first);
        CHECK(newData[i + mockSolutionRank0.size()].second == data.getRow(mockSolutionRank1[i].first));
    }
}