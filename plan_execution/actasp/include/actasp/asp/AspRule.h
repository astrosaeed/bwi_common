#pragma once

#include <vector>
#include <string>
#include <sstream>
#include <actasp/asp/AspElement.h>
#include <actasp/asp/AspFunction.h>
#include <actasp/asp/AspFluent.h>
#include <actasp/asp/AspLiteral.h>

namespace actasp {

/**
 * @brief The simplest combination of atoms
 * Ex: i_am <- i_think().
 */
struct AspRule : public AspElement {

  const LiteralContainer head;
  const LiteralContainer body;

  AspRule() : head(), body() {}

  ~AspRule() override = default;

  AspRule(const LiteralContainer &head, const LiteralContainer &body) : head(head), body(body) {}


  static AspRule with_atoms(std::vector<AspAtom> head, std::vector<AspAtom> body) {
    LiteralContainer head_t;
    LiteralContainer body_t;
    for (const auto &atom: head) {
      head_t.push_back(new AtomLiteral(atom));
    }
    for (const auto &atom: body) {
      body_t.push_back(new AtomLiteral(atom));
    }
    return {head_t, body_t};
  }

  static AspRule with_fluents(std::vector<AspFluent> head, std::vector<AspFluent> body) {
    LiteralContainer head_t;
    LiteralContainer body_t;
    for (const auto &atom: head) {
      head_t.push_back(new FluentLiteral(atom));
    }
    for (const auto &atom: body) {
      body_t.push_back(new FluentLiteral(atom));
    }
    return {head_t, body_t};
  }

  AspRule(const AspRule &other) : head(other.head), body(other.body) {

  }

  bool operator==(const AspRule &other) const noexcept {
    return this->head == other.head && this->body == other.body;
  }

  std::vector<AspFluent> head_fluents() const {
    std::vector<AspFluent> fluents;
    for (const auto &literal: body) {
      if (const auto fluent = dynamic_cast<const AspFluent *>(&literal)) {
        fluents.push_back(*fluent);
      }
    }
    return fluents;
  }

  std::vector<AspFluent> body_fluents() const {
    std::vector<AspFluent> fluents;
    for (const auto &literal: body) {
      if (const auto fluent = dynamic_cast<const AspFluent *>(&literal)) {
        fluents.push_back(*fluent);
      }
    }
    return fluents;
  }

  static AspRule fact(const LiteralContainer &head) {
    return AspRule(head, {});
  }

  static AspRule fact(const AspLiteral &head) {
    return AspRule(literal_to_container(head), {});
  }

  /**
 * @brief Convenience constructor for the common single-atom case.
 * @param head
 */
  static AspRule fact(const AspAtom &head) {
    return AspRule(literal_to_container(AtomLiteral(head)), {});
  }

  static AspRule fact(const AspFluent &head) {
    return AspRule(literal_to_container(FluentLiteral(head)), {});
  }

  /**
 * @brief A convenience wrapper for AspRule that only has a body.
 * Ex:      :- a, b.
 */
  static AspRule integrity_constraint(const LiteralContainer &body) {
    return AspRule({}, body);
  }

  static AspRule integrity_constraint(const AtomLiteral &first, const AtomLiteral &second) {
    return AspRule({}, literals_to_container(std::vector<AtomLiteral>{first, second}));
  }

  static AspRule integrity_constraint(const std::vector<AtomLiteral> &body) {
    return AspRule({}, literals_to_container(body));
  }


};

inline AspRule operator<=(const LiteralContainer &head, const LiteralContainer &body) {
  return AspRule(head, body);
}

typedef AspRule AspFact;
typedef AspRule AspIntegrityConstraint;
}