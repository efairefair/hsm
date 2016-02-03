#include "myAtomicInt.h"

myAtomicInt::myAtomicInt() {
    data=0;
}

myAtomicInt::myAtomicInt(long long int initialValue) {
    data=initialValue;
}

myAtomicInt::myAtomicInt(const myAtomicInt & srcValue) {
    long long int temp=srcValue.data;
    data=temp;
}

myAtomicInt& myAtomicInt::operator=( const myAtomicInt& rhs ) {

    long long int temp=rhs.data;
    this->data=temp;
    return *this;

}

//std::ostream& ostream::operator<<(const myAtomicInt& obj) {
//    os << obj.data;
//    return os;
//}
