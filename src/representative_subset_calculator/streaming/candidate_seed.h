


class CandidateSeed {
    private:
    std::vector<double> data;
    unsigned int globalRow;
    unsigned int originRank;

    public:
    CandidateSeed(
        const unsigned int row, 
        std::vector<double> data,
        const unsigned int rank
    ) : 
        data(move(data)), 
        globalRow(row),
        originRank(rank)
    {}

    const std::vector<double> &getData() {
        return this->data;
    }

    unsigned int getRow() {
        return this->globalRow;
    }

    unsigned int getOriginRank() {
        return this->originRank;
    }
};
