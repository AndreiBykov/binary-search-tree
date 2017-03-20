#include <iostream>
#include <pthread.h>
#include <sys/time.h>

class BinaryTreeEntry {

public:
    BinaryTreeEntry(int value) : value(value) {}

    int value;

    BinaryTreeEntry *left;
    BinaryTreeEntry *right;

    void addEntry(BinaryTreeEntry *entry) {

        if (entry == nullptr)
            return;

        if (entry->value > this->value) {
            if (right != nullptr)
                right->addEntry(entry);
            else
                right = entry;

        } else if (entry->value < this->value) {
            if (left != nullptr)
                left->addEntry(entry);
            else
                left = entry;
        }

    }

    ~BinaryTreeEntry() {
        if (left != nullptr)
            delete left;
        if (right != nullptr)
            delete right;
    }

};

class BinaryTree {

private:
    BinaryTreeEntry *root;
    pthread_mutex_t mutex;

    void removeEntry(BinaryTreeEntry *entry) {
        entry->left = nullptr;
        entry->right = nullptr;
        delete entry;
    }

public:
    BinaryTree() {
        pthread_mutex_init(&mutex, NULL);
    }

    bool add(int value) {
        pthread_mutex_lock(&mutex);
        if (root == nullptr) {
            root = new BinaryTreeEntry(value);
            pthread_mutex_unlock(&mutex);
            return true;
        }

        BinaryTreeEntry *temp = root;
        BinaryTreeEntry *parent;
        do {
            parent = temp;
            if (value > temp->value)
                temp = temp->right;
            else if (value < temp->value)
                temp = temp->left;
            else {
                pthread_mutex_unlock(&mutex);
                return false;
            }
        } while (temp != nullptr);

        BinaryTreeEntry *newEntry = new BinaryTreeEntry(value);

        if (value > parent->value)
            parent->right = newEntry;
        else
            parent->left = newEntry;
        pthread_mutex_unlock(&mutex);
        return true;

    }

    bool remove(int value) {
        pthread_mutex_lock(&mutex);
        BinaryTreeEntry *temp = root;

        if (temp == nullptr) {
            pthread_mutex_unlock(&mutex);
            return false;
        }

        if (temp->value == value) {
            if (temp->left != nullptr) {
                root = temp->left;
                root->addEntry(temp->right);
            } else
                root = temp->right;
            removeEntry(temp);
            pthread_mutex_unlock(&mutex);
            return true;
        }

        BinaryTreeEntry *parent;
        do {
            parent = temp;
            if (value > temp->value)
                temp = temp->right;
            else if (value < temp->value)
                temp = temp->left;
        } while (temp != nullptr && temp->value != value);

        if (temp == nullptr) {
            pthread_mutex_unlock(&mutex);
            return false;
        }

        if (parent->left == temp) {
            if (temp->left != nullptr) {
                parent->left = temp->left;
                parent->left->addEntry(temp->right);
            } else
                parent->left = temp->right;
        } else {
            if (temp->left != nullptr) {
                parent->right = temp->left;
                parent->right->addEntry(temp->right);
            } else
                parent->right = temp->right;
        }

        removeEntry(temp);
        pthread_mutex_unlock(&mutex);
        return true;
    }

    bool contains(int value) {
        pthread_mutex_lock(&mutex);
        BinaryTreeEntry *temp = root;

        while (temp != nullptr) {
            if (value > temp->value)
                temp = temp->right;
            else if (value < temp->value)
                temp = temp->left;
            else {
                pthread_mutex_unlock(&mutex);
                return true;
            }
        }
        pthread_mutex_unlock(&mutex);
        return false;
    }

    void removeAll() {
        pthread_mutex_lock(&mutex);
        if (root != nullptr) {
            delete root;
            root = nullptr;
        }
        pthread_mutex_unlock(&mutex);
    }

    ~BinaryTree() {
        pthread_mutex_destroy(&mutex);
        if (root != nullptr)
            delete root;
    }
};

BinaryTree *tree;
pthread_barrier_t barrier;
pthread_once_t timer_start_once = PTHREAD_ONCE_INIT,
        timer_end_once = PTHREAD_ONCE_INIT;

struct timeval start, end;
double elapsed_time;

void timer_start(void) { gettimeofday(&start, NULL); }

void timer_end(void) { gettimeofday(&end, NULL); }

void *testFunc(void *n) {
    long number = (long) n;

    pthread_barrier_wait(&barrier);
    pthread_once(&timer_start_once, timer_start);

    for (int i = 0; i < number; ++i) {
        switch (rand() % 3) {
            case 0:
                tree->add(rand() % 10000);
                break;
            case 1:
                tree->contains(rand() % 10000);
                break;
            case 2:
                tree->remove(rand() % 10000);
                break;
            default:
                break;
        }
    }

    pthread_barrier_wait(&barrier);
    pthread_once(&timer_end_once, timer_end);
    pthread_exit(NULL);
}

int main(int argc, char **argv) {
    if (argc < 3) {
        std::cout << "Not enough args" << std::endl;
        return 1;
    }

    int numberOfOperations = atoi(argv[1]);
    int numberOfThreads = atoi(argv[2]);

    std::cout << "Binary search tree via mutex" << std::endl;
    std::cout << "operations: " << numberOfOperations << "\tthreads: " << numberOfThreads << std::endl;
    srand((unsigned int) time(NULL));

    tree = new BinaryTree();

    pthread_t threads[numberOfThreads];
    pthread_barrier_init(&barrier, NULL, (unsigned int) numberOfThreads);

    for (int i = 0; i < numberOfThreads; ++i) {
        long number = numberOfOperations / (numberOfThreads - i);
        numberOfOperations -= number;
        pthread_create(&threads[i], NULL, testFunc, (void *) number);
    }

    for (int i = 0; i < numberOfThreads; ++i) {
        pthread_join(threads[i], NULL);
    }

    pthread_barrier_destroy(&barrier);

    double elapsed_time = (end.tv_sec - start.tv_sec) + (double) (end.tv_usec - start.tv_usec) / 1000000;
    std::cout << "Result time: " << elapsed_time << " s" << std::endl;

    return 0;
}