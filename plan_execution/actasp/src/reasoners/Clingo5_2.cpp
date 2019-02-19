#include <actasp/reasoners/Clingo5_2.h>

#include <actasp/asp/AspAggregate.h>
#include <actasp/asp/AspRule.h>
#include <actasp/AnswerSet.h>
#include <actasp/asp/AspFunction.h>
#include <actasp/asp/AspProgram.h>
#include <actasp/asp/AspMeta.h>
#include <actasp/asp/AspMinimize.h>
#include <actasp/action_utils.h>

#include <algorithm>
#include <set>
#include <iterator>
#include <sstream>
#include <fstream>
#include <cstdlib>
#include <iostream>
#include <limits>

#include <boost/filesystem.hpp>
#include <actasp/filesystem_utils.h>
#include <clingo.hh>

using namespace std::placeholders;
using namespace std;
using boost::filesystem::path;

namespace actasp {


string literal_to_string(const AspLiteral &literal) {
  if (const auto downcast = dynamic_cast<const AtomLiteral*>(&literal)) {
    return downcast->to_string();
  } else if (const auto downcast = dynamic_cast<const FluentLiteral*>(&literal)) {
    return downcast->to_string();
  } else if (const auto downcast = dynamic_cast<const BinRelation*>(&literal)) {
    return downcast->to_string();
  } else {
    assert(false);
  }
}

string rule_to_string(const AspRule *rule) {
  stringstream sstream;
  bool first = true;
  for (const auto &literal: rule->head) {
    if (!first) {
      sstream << ", ";
    }
    sstream << literal_to_string(literal);
    first = false;
  }
  if (!rule->body.empty()) {
    sstream << " :- ";
  }
  first = true;
  for (const auto &literal: rule->body) {
    if (!first) {
      sstream << ", ";
    }
    sstream << literal_to_string(literal);
    first = false;
  }
  sstream << ".";
  return sstream.str();
}

// TODO: Support minimization statements
string minimize_to_string(const AspMinimize *minimize) {
  assert(false);
}

// TODO: Support meta statements
string meta_to_string(const AspMeta *minimize) {
  assert(false);
}

string element_to_string(const AspElement *element) {
  if (const auto r = dynamic_cast<const AspRule*>(element)) {
    return rule_to_string(r);
  } else if (const auto c = dynamic_cast<const AspMinimize*>(element)) {
    return minimize_to_string(c);
  } else if (const auto m = dynamic_cast<const AspMeta*>(element)) {
    return meta_to_string(m);
  } else {
  // Note: Atoms and fluents don't belong at the top level of an ASP program!
  // These must at least be a part of an AspFact.
  assert(false);
  }
}

string Clingo5_2::program_to_string(const AspProgram &program) {
  stringstream stream;
  stream << "#program " << program.name;
  if (!program.variables.empty()) {
    stream << "(";
    bool past_first = false;
    for (const auto &variable: program.variables) {
      if (past_first) {
        stream << ", ";
      }
      stream << variable;
      past_first = true;
    }
    stream << ")";
  }
  stream << "." << std::endl;
  for (const auto &element: program.elements) {
    stream << element_to_string(element);
  }
  return stream.str();
}

static AnswerSet model_to_answer_set(const Clingo::Model &model, const std::set<string> &fluent_names) {

  vector<AspFluent> fluents;
  vector<AspAtom> atoms;
  for (auto &atom : model.symbols(Clingo::ShowType::All)) {
      if (atom.type() == Clingo::SymbolType::Function) {
        const auto parsed = AspAtom::from_symbol(atom);
        if (fluent_names.find(atom.name()) != fluent_names.end()) {
          fluents.emplace_back(parsed.name, parsed.arguments, parsed.negative);
        } else {
          atoms.push_back(parsed);
        }
      } else {
        assert(false);
      }
  }
  return AnswerSet(atoms, fluents);
}

static Plan
model_to_plan(const Clingo::Model &model, const std::set<string> &fluent_names, const std::set<string> &action_names) {
  vector<AspFluent> fluents;
  vector<AspFluent> actions;
  vector<AspAtom> atoms;
  for (auto &atom : model.symbols(Clingo::ShowType::All)) {
    if (atom.type() == Clingo::SymbolType::Function) {
      const auto parsed = AspAtom::from_symbol(atom);
      if (action_names.find(atom.name()) != action_names.end()) {
        actions.emplace_back(parsed.name, parsed.arguments, parsed.negative);
      } else if (fluent_names.find(atom.name()) != fluent_names.end()) {
        fluents.emplace_back(parsed.name, parsed.arguments, parsed.negative);
      } else {
        atoms.push_back(parsed);
      }
    } else {
      assert(false);
    }
  }
  return Plan(atoms, fluents, actions);
}

struct PlanCollector {
  vector<Plan> plans;
  const set<string> &fluent_names;
  const set<string> &action_names;

  PlanCollector(const set<string> &fluent_names, const set<string> &action_names): fluent_names(fluent_names), action_names(action_names) {}

  void model_cb(const Clingo::Model &model) {
    const auto plan = model_to_plan(model, fluent_names, action_names);
    plans.push_back(plan);

  };
};

struct AnswerSetCollector {
  vector<AnswerSet> sets;
  const set<string> &fluent_names;

  explicit AnswerSetCollector(const set<string> &fluent_names): fluent_names(fluent_names) {}

  void model_cb(const Clingo::Model &model) {
    const auto plan = model_to_answer_set(model, fluent_names);
    sets.push_back(plan);
  };
};



static inline void add(Clingo::Control &control, const AspProgram &program) {
  vector<const char *> as_strings;
  for (const auto &variable: program.variables) {
    as_strings.push_back(variable.name.c_str());
  }
  stringstream stream;
  // TODO: Add this external statement only where needed as part of the program...
  stream << "#external query(n)." << std::endl;
  // TODO: Replace string based add with program building add.
  // This should let us do away with some of the custom string processing, or at least
  // avoid that unecessary step here.
  for (const auto &rule: program.elements) {
    stream << element_to_string(rule);
  }
  const auto program_str = stream.str();
  control.add(program.name.c_str(), as_strings, program_str.c_str());
}

template<typename E>
static vector<AspElement *> make_unsafe_pointer_vector(const vector<E> &elements) {
  vector<AspElement *> upcast;
  for (const auto &e: elements) {
    upcast.push_back((AspElement *) (&e));
  }
  return upcast;
}

static vector<AspFact> plan_as_facts(const Plan &plan) {
  vector<AspFact> facts;
  for (const auto &action: plan.actions) {
    facts.emplace_back(AspFact::fact(action));
  }
  return facts;
}

Clingo5_2::Clingo5_2(
    const vector<string> &domain_files
) noexcept :
    domain_files(domain_files),
    incremental_var("n") {

  // FIXME: Just parse the program here
  //  We don't really need to ground, but we don't support pooling yet so we rely on grounding to
  //  bake the annotations down to atoms.
  Clingo::Control control;
  for (const auto &linkFile: domain_files) {
    assert(boost::filesystem::is_regular_file(linkFile));
    control.load(linkFile.c_str());
  }
  control.ground({{"base", Clingo::SymbolSpan{}}});
  for (const auto &atom: control.symbolic_atoms()) {
    const auto symbol = atom.symbol();
    // Domains can annotate which predicates denote actions using the "action" predicate with the name of the action
    // as the sole string argument
    if (symbol.type() == Clingo::SymbolType::Function) {
      if (symbol.name() == string("action")) {
        const string value = symbol.arguments()[0].string();
        action_names.insert(value);
        fluent_names.insert(value);
      } else if (symbol.name() == string("fluent")) {
        const string value = symbol.arguments()[0].string();
        fluent_names.insert(value);
      }
    }
  }
  if (control.has_const("incvar")) {
    incremental_var = control.get_const("incvar").string();
  }

}


vector<Plan> Clingo5_2::minimalPlanQuery(const vector<AspRule> &goal,
                                         unsigned int max_plan_length,
                                         unsigned int answerset_number,
                                         const vector<AspFact> *knowledge) const noexcept {

  vector<AspProgram> programs;
  programs.emplace_back("check", "minimal_plan_query", make_unsafe_pointer_vector<AspRule>(goal), vector<Variable>{Variable("n")});
  if (knowledge) {
    programs.emplace_back("base", "knowledge", make_unsafe_pointer_vector<AspFact>(*knowledge));
  }

  PlanCollector collector(fluent_names, action_names);
  makeQuery(programs, bind(&PlanCollector::model_cb, &collector, _1), 0, max_plan_length, answerset_number, true);
  if (log_queries) {
    log_query<Plan>(programs, collector.plans);
  }
  return collector.plans;

}

vector<actasp::Plan> Clingo5_2::lengthRangePlanQuery(const vector<AspRule> &goal,
                                                     unsigned int min_plan_length,
                                                     unsigned int max_plan_length,
                                                     unsigned int answerset_number,
                                                     const vector<AspFact> *knowledge) const noexcept {
  vector<AspProgram> programs;
  programs.emplace_back("check", "length_range_plan_query", make_unsafe_pointer_vector<AspRule>(goal), vector<Variable>{Variable("n")});
  if (knowledge) {
    programs.emplace_back("base", "knowledge", make_unsafe_pointer_vector<AspFact>(*knowledge));
  }
  PlanCollector collector(fluent_names, action_names);
  makeQuery(programs, bind(&PlanCollector::model_cb, &collector, _1), min_plan_length, max_plan_length,
                                                    answerset_number, false);
  if (log_queries) {
    log_query<Plan>(programs, collector.plans);
  }
  return collector.plans;
}

actasp::Plan Clingo5_2::optimalPlanQuery(const vector<AspRule> &goal,
                                         unsigned int max_plan_length,
                                         unsigned int answerset_number,
                                         const vector<AspFact> *knowledge) const noexcept {
  vector<AspProgram> programs;
  programs.emplace_back("check", "optimal_plan_query", make_unsafe_pointer_vector<AspRule>(goal), vector<Variable>{Variable("n")});
  if (knowledge) {
    programs.emplace_back("base", "knowledge", make_unsafe_pointer_vector<AspFact>(*knowledge));
  }

  PlanCollector collector(fluent_names, action_names);
  makeQuery(programs, bind(&PlanCollector::model_cb, &collector, _1), 0, max_plan_length, answerset_number);
  if (log_queries) {
    log_query<Plan>(programs, collector.plans);
  }
  return collector.plans.front();
}

AnswerSet Clingo5_2::currentStateQuery(const vector<AspRule> &query) const noexcept {
  assert(false);
  // TODO: This used to be aspString(query, 0). Fix it
//  list<AnswerSet> sets = makeQuery(query, 0, 0, "stateQuery", 1, nullptr);

//  return (sets.empty()) ? AnswerSet() : *(sets.begin());
}

vector<AnswerSet>
Clingo5_2::genericQuery(const vector<AspProgram> &programs,
                        unsigned int min_plan_length,
                        unsigned int max_plan_length,
                        unsigned int max_num_plans,
                        bool shortest_only) const noexcept {
  return genericQuery(programs, min_plan_length, max_plan_length, max_num_plans, shortest_only, nullptr);
}

vector<AnswerSet> Clingo5_2::genericQuery(const vector<AspProgram> &programs,
                                          unsigned int min_plan_length,
                                          unsigned int max_plan_length,
                                          unsigned int max_num_plans,
                                          bool shortest_only,
                                          std::vector<AspFact> *knowledge) const noexcept {


  AnswerSetCollector collector(fluent_names);
  makeQuery(programs, bind(&AnswerSetCollector::model_cb, &collector, _1), min_plan_length, max_plan_length, max_num_plans, shortest_only);
  if (log_queries) {
    log_query<AnswerSet>(programs, collector.sets);
  }

  return collector.sets;

}

vector<actasp::Plan> Clingo5_2::monitorQuery(const vector<AspRule> &goal,
                                             const Plan &plan, const vector<AspFact> *knowledge) const noexcept {

  //   clock_t kr1_begin = clock();
  vector<AspProgram> programs;
  programs.emplace_back("check", "monitor_query", make_unsafe_pointer_vector<AspRule>(goal), vector<Variable>{Variable("n")});
  if (knowledge) {
    programs.emplace_back("base", "knowledge", make_unsafe_pointer_vector<AspFact>(*knowledge));
  }
  auto plan_facts = plan_as_facts(plan);
  programs.emplace_back("base", "plan_as_facts", make_unsafe_pointer_vector<AspFact>(plan_facts));


  PlanCollector collector(fluent_names, action_names);
  makeQuery(programs, bind(&PlanCollector::model_cb, &collector, _1), plan.actions.size(), plan.actions.size(), 1);
  if (log_queries) {
    log_query<Plan>(programs, collector.plans);
  }
  return collector.plans;

//   clock_t kr1_end = clock();
//   cout << "Verifying plan time: " << (double(kr1_end - kr1_begin) / CLOCKS_PER_SEC) << " seconds" << endl;
}

vector<Plan>
Clingo5_2::filteringQuery(const AnswerSet &currentState, const Plan &plan, const vector<AspRule> &goal,
                          const vector<AspFact> *knowledge) const noexcept {

  vector<AspProgram> programs;
  programs.emplace_back("check", make_unsafe_pointer_vector<AspRule>(goal), vector<Variable>{Variable("n")});
  if (knowledge) {
    programs.emplace_back("base", make_unsafe_pointer_vector<AspFact>(*knowledge));
  }
  auto plan_facts = plan_as_facts(plan);
  programs.emplace_back("base", make_unsafe_pointer_vector<AspFact>(plan_facts));
  assert(false);
  // TODO: Actually put the filtering back in
  //   generate a string with all the fluents "0{fluent}1."
  //   and add the minimize statement ( eg: :~ pos(x,y,z), ... . [1@1] )
  for (const auto &fluent: plan.actions) {
    TermContainer terms;
    terms.push_back(new AspFluent(fluent));
    AspAggregate(0, terms, 1);
    // TODO: Rewrite this in terms of minimize statement?
    //minimizeString << ":~ " << fluent->to_string() << ". [1]" << endl;
  }
  PlanCollector collector(fluent_names, action_names);
  makeQuery(programs, bind(&PlanCollector::model_cb, &collector, _1), plan.fluents.size(), plan.fluents.size(), 0);
  return collector.plans;

}


static bool
inc_solve_loop(Clingo::Control &control, Clingo5_2::ModelCallback model_cb, uint32_t initial_step, uint32_t final_step, bool shortest_only) {
  using Clingo::Number;
  using Clingo::Function;
  using Clingo::TruthValue;
  using Clingo::Control;
  Clingo::SolveResult result;

  int step = 0;
  while (step < final_step && (step == 0 || step < initial_step || !result.is_satisfiable())) {
    try {
      vector<Clingo::Part> parts;
      parts.emplace_back("check", Clingo::SymbolSpan{Number(step)});
      if (step == 0) {
        parts.emplace_back("base", Clingo::SymbolSpan{});
      } else {
        parts.push_back({"step", {Number(step)}});
        control.cleanup();
      }
      control.ground(parts);
      if (step == 0) {
        control.assign_external(Function("query", {Number(step)}), TruthValue::True);
      } else {
        control.release_external(Function("query", {Number(step - 1)}));
        control.assign_external(Function("query", {Number(step)}), TruthValue::True);
      }
      /*
      for (auto atom: control.symbolic_atoms()) {
        std::cout << atom.symbol() << std::endl;
      }
      std::cout << "_------ END PROGRAM" << std::endl;*/
      step++;
      // We don't care about models outside our desired time steps
      if (step < initial_step) {
        continue;
      }
      auto handle = control.solve(Clingo::LiteralSpan{}, nullptr,false, true);
      result = handle.get();
      //std::cout << handle.get() << std::endl;
      for (const auto &model: handle) {
        model_cb(model);
      }
      if (result.is_satisfiable() && shortest_only) {
        return true;
      }
    }
    catch (exception const &e) {
      cerr << "Solving failed with: " << e.what() << endl;
      return false;
    }
  }
  return true;

}

/**
 * @brief Outputs via callback at most n=max_num_plans models, each if which is of length min_plan_length<=m<=max_plan_length
 * @param programs
 * @param model_cb the function which will receive the model
 * @param min_plan_length
 * @param max_plan_length
 * @param max_num_plans
 * @param shortest_only
 * @return
 */
void Clingo5_2::makeQuery(const vector<AspProgram> &programs,
                     ModelCallback model_cb,
                     unsigned int min_plan_length,
                     unsigned int max_plan_length,
                     unsigned int max_num_plans,
                     bool shortest_only) const noexcept {
  // TODO: Expose a way to send this output to the logs
  //   Atleast a way to turn it off...
  auto logger = [](Clingo::WarningCode, char const *message) {
    cerr << message << endl;
  };

  Clingo::Control control({}, logger, 20);

  control.configuration()["solve"]["models"] = std::to_string(max_num_plans).c_str();

  for (const auto &path: domain_files) {
    control.load(path.c_str());
  }
  for (const auto &program: programs) {
    add(control, program);
  }

  inc_solve_loop(control, std::move(model_cb), min_plan_length + 1, max_plan_length + 1, shortest_only);


}


}
