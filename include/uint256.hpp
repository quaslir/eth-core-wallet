#include <openssl/bn.h>
#include <string>
#include <memory>
#include <stdexcept> 

    struct BNDeleter {
        void operator()(BIGNUM*n) const{
            BN_free(n);
        }
    };

    using unique_bignum = std::unique_ptr<BIGNUM, BNDeleter>;

class Uint256 {
    private:



    unique_bignum bn;


    public:

    Uint256();
    Uint256(const Uint256& num);
    Uint256(const std::string& str, bool hex);
    Uint256(Uint256&& num) noexcept;
    Uint256& operator=(const Uint256& num);
    Uint256& operator=(Uint256&& num) noexcept;
    bool operator==(const Uint256& num) const;
    bool operator<(const Uint256& num) const;
    bool operator>(const Uint256& num) const;
    bool operator<=(const Uint256& num) const;
    bool operator>=(const Uint256& num) const;


    Uint256& operator+=(const Uint256& num) ;
    Uint256 operator+(const Uint256& num)const ;
    std::string from_wei_to_eth(void) const;
};