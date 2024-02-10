#include "orchestrator.h"
#include <mpi.h>

class MpiOrchestrator : public Orchestrator {

    public:
    static nlohmann::json getTimersFromMachines(
        const Timers &localTimers,
        const int worldRank,
        const int worldSize
    ) {
        nlohmann::json localTimerJson = localTimers.outputToJson();
        std::ostringstream outputStream;
        outputStream << localTimerJson.dump(2);
        std::string localTimeData(outputStream.str());

        // send timer string lengths
        int localSendSize = localTimeData.size();
        std::vector<int> receiveSizes(worldSize, 0);
        MPI_Gather(&localSendSize, 1, MPI_INT, receiveSizes.data(), 1, MPI_INT, 0, MPI_COMM_WORLD);

        int receiveTotal = 0;
        std::vector<int> displacements(worldSize, 0);
        for (size_t rank = 0; rank < worldSize; rank++) {
            displacements[rank] = receiveTotal;
            receiveTotal += receiveSizes[rank];
        }

        std::vector<char> receiveBuffer(receiveTotal);

        // send timer strings
        MPI_Gatherv(
            localTimeData.data(), 
            localSendSize, 
            MPI_CHAR, 
            receiveBuffer.data(), 
            receiveSizes.data(), 
            displacements.data(), 
            MPI_CHAR, 
            0,
            MPI_COMM_WORLD
        );

        std::string timerData(receiveBuffer.begin(), receiveBuffer.end());

        nlohmann::json output;
        for (size_t rank = 0; rank < worldSize; rank++) {
            const size_t end = rank == worldSize - 1 ? timerData.size() : displacements[rank + 1];
            output.push_back(timerData.substr(
                displacements[rank],
                end - displacements[rank]
            ));
        }

        // return as json
        return output;
    }

    static nlohmann::json buildMpiOutput(
        const AppData &appData, 
        const RepresentativeSubset &solution,
        const Data &data,
        const Timers &timers
    ) {
        nlohmann::json output = Orchestrator::buildOutputBase(appData, solution, data, timers);

        output.push_back({"timers", getTimersFromMachines(timers, appData.worldRank, appData.worldSize)});

        return output;
    }
};