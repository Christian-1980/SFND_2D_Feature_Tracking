#include <iostream>
#include <vector>
#include <opencv2/opencv.hpp>


// Dummy DataFrame class for testing in main()
class DataFrame {
public:
    DataFrame(const std::string& name) : name_(name) {}
    std::string getName() const { return name_; }
private:
    std::string name_;
};


class ImageRingBuffer {
public:
    ImageRingBuffer(size_t capacity) : capacity_(capacity), head_(0), tail_(0), count_(0) {
        buffer_.resize(capacity_);
    }

    void push(const std::vector<DataFrame>& dataFrames) {
        if (count_ == capacity_) {
            // Buffer is full, overwrite the oldest element
            tail_ = (tail_ + 1) % capacity_;
        } else {
            count_++;
        }
        buffer_[head_] = dataFrames;
        head_ = (head_ + 1) % capacity_;
    }

    bool pop(std::vector<DataFrame>& dataFrames) {
        if (count_ == 0) {
            // Buffer is empty
            return false;
        }
        dataFrames = buffer_[tail_];
        tail_ = (tail_ + 1) % capacity_;
        count_--;
        return true;
    }

    size_t size() const {
        return count_;
    }

    size_t capacity() const {
        return capacity_;
    }

    bool isEmpty() const{
        return count_ == 0;
    }

    bool isFull() const{
        return count_ == capacity_;
    }

private:
    std::vector<std::vector<DataFrame>> buffer_;
    size_t head_;
    size_t tail_;
    size_t capacity_;
    size_t count_;
};

// // Example Usage
// int main() {
//     ImageRingBuffer ringBuffer(3);

//     std::vector<DataFrame> dataFrames1;
//     dataFrames1.emplace_back("DataFrame1");
//     dataFrames1.emplace_back("DataFrame2");

//     std::vector<DataFrame> dataFrames2;
//     dataFrames2.emplace_back("DataFrame3");

//     ringBuffer.push(dataFrames1);
//     ringBuffer.push(dataFrames2);

//     std::vector<DataFrame> retrievedDataFrames;
//     if (ringBuffer.pop(retrievedDataFrames)) {
//         std::cout << "Retrieved " << retrievedDataFrames.size() << " DataFrames." << std::endl;
//         for(const auto& df : retrievedDataFrames){
//             std::cout << "DataFrame name: " << df.getName() << std::endl;
//         }
//     }

//     return 0;
// }