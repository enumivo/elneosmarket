#include "ex.hpp"

#include <cmath>
#include <enulib/action.hpp>
#include <enulib/asset.hpp>
#include "enu.token.hpp"

using namespace enumivo;
using namespace std;

void ex::buy(const currency::transfer &transfer) {
  if (transfer.to != _self) {
    return;
  }

  // get EOS supply
  double eos_supply = enumivo::token(N(stable.coin)).
	   get_supply(enumivo::symbol_type(EOS_SYMBOL).name()).amount;

  eos_supply = eos_supply/10000;

  //y = 100k / sqrt(x)
  double buy = pow((received/200000)+sqrt(eos_supply),2)-eos_supply;

  auto to = transfer.from;

  auto quantity = asset(10000*buy, EOS_SYMBOL);

  action(permission_level{_self, N(active)}, N(stable.coin), N(issue),
         std::make_tuple(to, quantity,
                         std::string("Create EOS")))
      .send();
}

void ex::sell(const currency::transfer &transfer) {
  if (transfer.to != _self) {
    return;
  }

    // get EOS supply
  double eos_supply = enumivo::token(N(stable.coin)).
	   get_supply(enumivo::symbol_type(EOS_SYMBOL).name()).amount;

  eos_supply = eos_supply/10000;

  //y = 100k / sqrt(x)
  double sell = 200000*(sqrt(eos_supply)-sqrt(eos_supply-received));

  auto to = transfer.from;

  auto quantity = asset(10000*sell, ELN_SYMBOL);

  action(permission_level{_self, N(active)}, N(eln.coin), N(transfer),
         std::make_tuple(_self, to, quantity,
                         std::string("Sell EOS for ELN")))
      .send();

  auto toretire = asset(10000*received, EOS_SYMBOL);

  action(permission_level{_self, N(active)}, N(stable.coin), N(retire),
         std::make_tuple(toretire, std::string("Destroy EOS")))
      .send();
}

void ex::apply(account_name contract, action_name act) {
  if (contract == N(eln.coin) && act == N(transfer)) {
    auto transfer = unpack_action_data<currency::transfer>();
    enumivo_assert(transfer.quantity.symbol == ELN_SYMBOL,
                 "Must send ELN");
    buy(transfer);
    return;
  }

  if (contract == N(stable.coin) && act == N(transfer)) {
    auto transfer = unpack_action_data<currency::transfer>();
    enumivo_assert(transfer.quantity.symbol == EOS_SYMBOL,
                 "Must send EOS");
    sell(transfer);
    return;
  }

  if (act == N(transfer)) {
    auto transfer = unpack_action_data<currency::transfer>();
    enumivo_assert(false, "Must send EOS or ELN");
    sell(transfer);
    return;
  }

  if (contract != _self) return;
}

extern "C" {
[[noreturn]] void apply(uint64_t receiver, uint64_t code, uint64_t action) {
  ex elneos(receiver);
  elneos.apply(code, action);
  enumivo_exit(0);
}
}
