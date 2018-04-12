#include "pch.h"
#include <dplyr/main.h>

#include <dplyr/visitor_set/VisitorSetIndexSet.h>

#include <dplyr/RowwiseDataFrame.h>
#include <dplyr/MultipleVectorVisitors.h>
#include <dplyr/DataFrameSubsetVisitors.h>
#include <dplyr/Result/Count_Distinct.h>

using namespace Rcpp;
using namespace dplyr;

SEXP select_not_grouped(const DataFrame& df, const SymbolVector& keep, const SymbolVector& new_names);

// [[Rcpp::export]]
SEXP distinct_impl(DataFrame df, const IntegerVector& vars, const IntegerVector& keep) {
  if (df.size() == 0) {
    DataFrame res = DataFrame::create();
    copy_most_attributes(res, df);
    res.attr("row.names") = IntegerVector::create(NA_INTEGER, -1);
    return res ;
  }

  // No vars means ungrouped data with keep_all = TRUE.
  if (vars.size() == 0)
    return df;

  check_valid_colnames(df, true);
  DataFrameVisitors visitors(df, vars);

  std::vector<int> indices;
  VisitorSetIndexSet<DataFrameVisitors> set(visitors);

  int n = df.nrows();
  for (int i = 0; i < n; i++) {
    if (set.insert(i).second) {
      indices.push_back(i);
    }
  }

  return DataFrameSubsetVisitors(df, keep).subset(indices, get_class(df));
}

// [[Rcpp::export]]
SEXP n_distinct_multi(List variables, bool na_rm = false) {
  if (variables.length() == 0) {
    stop("Need at least one column for `n_distinct()`");
  }

  MultipleVectorVisitors visitors(variables);
  NaturalSlicingIndex everything(visitors.nrows());
  if (na_rm) {
    Count_Distinct_Narm<MultipleVectorVisitors> counter(visitors);
    return counter.process(everything);
  } else {
    Count_Distinct<MultipleVectorVisitors> counter(visitors);
    return counter.process(everything);
  }
}
