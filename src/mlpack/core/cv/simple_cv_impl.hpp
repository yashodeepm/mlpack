/**
 * @file simple_cv_impl.hpp
 * @author Kirill Mishchenko
 *
 * The implementation of the class SimpleCV.
 *
 * mlpack is free software; you may redistribute it and/or modify it under the
 * terms of the 3-clause BSD license.  You should have received a copy of the
 * 3-clause BSD license along with mlpack.  If not, see
 * http://www.opensource.org/licenses/BSD-3-Clause for more information.
 */
#ifndef MLPACK_CORE_CV_SIMPLE_CV_IMPL_HPP
#define MLPACK_CORE_CV_SIMPLE_CV_IMPL_HPP

#include <cmath>

namespace mlpack {
namespace cv {

template<typename MLAlgorithm,
         typename Metric,
         typename MatType,
         typename PredictionsType,
         typename WeightsType>
template<typename... CVBaseArgs>
SimpleCV<MLAlgorithm,
         Metric,
         MatType,
         PredictionsType,
         WeightsType>::SimpleCV(const double validationSize,
                                const CVBaseArgs&... args) :
    Base(args...)
{
  Init(validationSize, Base::ExtractDataArgs(args...));
}

template<typename MLAlgorithm,
         typename Metric,
         typename MatType,
         typename PredictionsType,
         typename WeightsType>
template<typename... MLAlgorithmArgs>
double SimpleCV<MLAlgorithm,
                Metric,
                MatType,
                PredictionsType,
                WeightsType>::Evaluate(const MLAlgorithmArgs&... args)
{
  return TrainAndEvaluate(args...);
}

template<typename MLAlgorithm,
         typename Metric,
         typename MatType,
         typename PredictionsType,
         typename WeightsType>
MLAlgorithm& SimpleCV<MLAlgorithm,
                      Metric,
                      MatType,
                      PredictionsType,
                      WeightsType>::Model()
{
  if (modelPtr == nullptr)
    throw std::logic_error(
        "SimpleCV::Model(): attempted to access an uninitialized model");

  return *modelPtr;
}

template<typename MLAlgorithm,
         typename Metric,
         typename MatType,
         typename PredictionsType,
         typename WeightsType>
template<typename DataArgsTupleT,
         typename>
void SimpleCV<MLAlgorithm,
              Metric,
              MatType,
              PredictionsType,
              WeightsType>::Init(const double validationSize,
                                 const DataArgsTupleT& dataArgsTuple)
{
  xs = std::get<0>(dataArgsTuple);
  ys = std::get<1>(dataArgsTuple);

  Base::AssertDataConsistency(xs, ys);

  InitTrainingAndValidationSets(validationSize);
}

template<typename MLAlgorithm,
         typename Metric,
         typename MatType,
         typename PredictionsType,
         typename WeightsType>
template<typename DataArgsTupleT,
         typename,
         typename>
void SimpleCV<MLAlgorithm,
              Metric,
              MatType,
              PredictionsType,
              WeightsType>::Init(const double validationSize,
                                 const DataArgsTupleT& dataArgsTuple)
{
  xs = std::get<0>(dataArgsTuple);
  ys = std::get<1>(dataArgsTuple);
  weights = std::get<2>(dataArgsTuple);

  Base::AssertDataConsistency(xs, ys, weights);

  InitTrainingAndValidationSets(validationSize);

  trainingWeights = GetSubset(weights, 0, trainingXs.n_cols - 1);
}

template<typename MLAlgorithm,
         typename Metric,
         typename MatType,
         typename PredictionsType,
         typename WeightsType>
void SimpleCV<MLAlgorithm,
              Metric,
              MatType,
              PredictionsType,
              WeightsType>::InitTrainingAndValidationSets(
    const double validationSize)
{
  size_t numberOfTrainingPoints = CalculateAndAssertNumberOfTrainingPoints(
      validationSize);

  trainingXs = GetSubset(xs, 0, numberOfTrainingPoints - 1);
  trainingYs = GetSubset(ys, 0, numberOfTrainingPoints - 1);

  validationXs = GetSubset(xs, numberOfTrainingPoints, xs.n_cols - 1);
  validationYs = GetSubset(ys, numberOfTrainingPoints, xs.n_cols - 1);
}

template<typename MLAlgorithm,
         typename Metric,
         typename MatType,
         typename PredictionsType,
         typename WeightsType>
size_t SimpleCV<MLAlgorithm,
                Metric,
                MatType,
                PredictionsType,
                WeightsType>::CalculateAndAssertNumberOfTrainingPoints(
    const double validationSize)
{
  if (validationSize < 0.0 || validationSize > 1.0)
    throw std::invalid_argument("SimpleCV: the validationSize parameter should "
        "be more than 0 and less than 1");

  if (xs.n_cols < 2)
    throw std::invalid_argument("SimpleCV: 2 or more data points are expected");

  size_t trainingPoints = round(xs.n_cols * (1.0 - validationSize));

  if (trainingPoints == 0 || trainingPoints == xs.n_cols)
    throw std::invalid_argument("SimpleCV: the validationSize parameter is "
        "either too small or too big");

  return trainingPoints;
}

template<typename MLAlgorithm,
         typename Metric,
         typename MatType,
         typename PredictionsType,
         typename WeightsType>
template<typename... MLAlgorithmArgs, bool Enabled, typename>
double SimpleCV<MLAlgorithm,
                Metric,
                MatType,
                PredictionsType,
                WeightsType>::TrainAndEvaluate(const MLAlgorithmArgs&... args)
{
  modelPtr = this->Train(trainingXs, trainingYs, args...);

  return Metric::Evaluate(*modelPtr, validationXs, validationYs);
}

template<typename MLAlgorithm,
         typename Metric,
         typename MatType,
         typename PredictionsType,
         typename WeightsType>
template<typename... MLAlgorithmArgs, bool Enabled, typename, typename>
double SimpleCV<MLAlgorithm,
                Metric,
                MatType,
                PredictionsType,
                WeightsType>::TrainAndEvaluate(const MLAlgorithmArgs&... args)
{
  if (trainingWeights.n_elem > 0)
    modelPtr = this->Train(trainingXs, trainingYs, trainingWeights, args...);
  else
    modelPtr = this->Train(trainingXs, trainingYs, args...);

  return Metric::Evaluate(*modelPtr, validationXs, validationYs);
}

} // namespace cv
} // namespace mlpack

#endif
