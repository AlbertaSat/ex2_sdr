//compile and then run using commands:
//
// pmt.dict(("bank":"gpio"), ("attr":"ATR_TX"), ("value":1), ("mask":1))

//    g++ -o sdr_ptt_program sdr_ptt_program.cc -lboost_program_options -luhd
//    ./sdr_ptt_program
//
//if issues arise, ensure UHD_IMAGES_DIR is configured properly by uhd, and ensure boost is installed on PC

#include <uhd/convert.hpp>
#include <uhd/usrp/multi_usrp.hpp>
#include <uhd/utils/safe_main.hpp>
#include <uhd/utils/thread.hpp>
#include <stdint.h>
#include <stdlib.h>
#include <boost/format.hpp>
#include <boost/program_options.hpp>
#include <boost/tokenizer.hpp>
#include <chrono>
#include <csignal>
#include <iostream>
#include <thread>

static const std::string GPIO_DEFAULT_GPIO       = "FP0";
static const size_t GPIO_DEFAULT_NUM_BITS        = 11;
static const std::string GPIO_DEFAULT_CTRL       = "0x0"; // all as user controlled
static const std::string GPIO_DEFAULT_DDR        = "0x0"; // all as inputs
static const std::string GPIO_DEFAULT_OUT        = "0x0";

static inline uint32_t GPIO_BIT(const size_t x)
{
    return (1 << x);
}

namespace po = boost::program_options;

std::string to_bit_string(uint32_t val, const size_t num_bits)
{
    std::string out;
    for (int i = num_bits - 1; i >= 0; i--) {
        std::string bit = ((val >> i) & 1) ? "1" : "0";
        out += "  ";
        out += bit;
    }
    return out;
}

void output_reg_values(const std::string& bank,
    const std::string& port,
    const uhd::usrp::multi_usrp::sptr& usrp,
    const size_t num_bits)
{
    const std::vector<std::string> attrs = {
        "CTRL", "DDR", "ATR_0X", "ATR_RX", "ATR_TX", "ATR_XX", "OUT", "READBACK"};
    std::cout << (boost::format("%10s:") % "Bit");
    for (int i = num_bits - 1; i >= 0; i--)
        std::cout << (boost::format(" %2d") % i);
    std::cout << std::endl;
    for (const auto& attr : attrs) {
        const uint32_t gpio_bits = uint32_t(usrp->get_gpio_attr(bank, attr));
        std::cout << (boost::format("%10s:%s") % attr
                         % to_bit_string(gpio_bits, num_bits))
                  << std::endl;
    }

    // GPIO Src - get_gpio_src() not supported for all devices
    try {
        // const auto gpio_src = usrp->get_gpio_src(port);
        // std::cout << boost::format("%10s:") % "SRC";
        // for (auto src : gpio_src) {
        //     std::cout << " " << src;
        // }
        // std::cout << std::endl;
    } catch (const uhd::not_implemented_error& e) {
        std::cout << "Ignoring " << e.what() << std::endl;
    } catch (...) {
        throw;
    }
}

int main(void){

    std::string ddr_str = GPIO_DEFAULT_DDR;
    std::string out_str = GPIO_DEFAULT_OUT;
    std::string src_str;
    std::string args;
    std::string gpio = GPIO_DEFAULT_GPIO;//removed default_value() function
    std::string port = port;
    size_t num_bits = GPIO_DEFAULT_NUM_BITS;


        // setup the program options
    po::options_description desc("Allowed options");
    // clang-format off
    desc.add_options()
        ("help", "help message")
        ("args", po::value<std::string>(&args)->default_value(""), "multi uhd device address args")
        ("list-banks", "print list of banks before running tests")
        ("bank", po::value<std::string>(&gpio)->default_value(GPIO_DEFAULT_GPIO), "name of gpio bank")
        ("port", po::value<std::string>(&port)->default_value(""), "name of gpio port. If not specified, defaults to the GPIO bank")
        ("bits", po::value<size_t>(&num_bits)->default_value(GPIO_DEFAULT_NUM_BITS), "number of bits in gpio bank")
        ("src", po::value<std::string>(&src_str), "GPIO SRC reg value")
        ("ddr", po::value<std::string>(&ddr_str)->default_value(GPIO_DEFAULT_DDR), "GPIO DDR reg value")
        ("out", po::value<std::string>(&out_str)->default_value(GPIO_DEFAULT_OUT), "GPIO OUT reg value")
    ;

    // uhd::device_addr_t dev_addr;
    // dev_addr = "31E54BA";
    // create a usrp device
    std::cout << std::endl;
    std::cout << boost::format("Creating the usrp device with: %s...") % args
              << std::endl;
    uhd::usrp::multi_usrp::sptr usrp = uhd::usrp::multi_usrp::make(args);//run gnuradio flow first
    std::cout << boost::format("Using Device: %s") % usrp->get_pp_string() << std::endl;

    //available gpio banks:
    std::cout << "Available GPIO banks: " << std::endl;
    auto banks = usrp->get_gpio_banks(0);
    for (auto& bank : banks) {
        std::cout << "* " << bank << std::endl;
    }

    std::cout << "Using GPIO bank: " << gpio << std::endl;

    // print out initial unconfigured state of GPIO
    std::cout << "Initial GPIO values:" << std::endl;
    output_reg_values(gpio, port, usrp, num_bits);

    // configure GPIO registers
    uint32_t ddr        = strtoul(ddr_str.c_str(), NULL, 0);
    uint32_t out        = strtoul(out_str.c_str(), NULL, 0);
    uint32_t ctrl       = 0;
    uint32_t atr_idle   = 0;
    uint32_t atr_rx     = 0;
    uint32_t atr_tx     = 0;
    uint32_t atr_duplex = 0;
    uint32_t mask       = (1 << num_bits) - 1;

    // GPIO[0] = ATR output 1 during TX
    ctrl |= GPIO_BIT(0);
    ddr |= GPIO_BIT(0);
    atr_tx |= GPIO_BIT(0);

    /*

    // set GPIO driver source (Not sure if necessary)
    if (vm.count("src")) {
        std::vector<std::string> gpio_src;
        typedef boost::char_separator<char> separator;
        boost::tokenizer<separator> tokens(src_str, separator(" "));
        std::copy(tokens.begin(), tokens.end(), std::back_inserter(gpio_src));
        usrp->set_gpio_src(port, gpio_src);
    }

    */

    // set data direction register (DDR)
    usrp->set_gpio_attr(gpio, "DDR", ddr, mask);

    // set control register
    usrp->set_gpio_attr(gpio, "CTRL", ctrl, mask);

    // set output values (OUT)
    usrp->set_gpio_attr(gpio, "OUT", out, mask);

    // set ATR registers
    usrp->set_gpio_attr(gpio, "ATR_0X", atr_idle, mask);
    usrp->set_gpio_attr(gpio, "ATR_RX", atr_rx, mask);
    usrp->set_gpio_attr(gpio, "ATR_TX", atr_tx, mask);
    usrp->set_gpio_attr(gpio, "ATR_XX", atr_duplex, mask);

    // print out initial state of FP GPIO
    std::cout << "\nConfigured GPIO values:" << std::endl;
    output_reg_values(gpio, port, usrp, num_bits);
    std::cout << std::endl;

}
