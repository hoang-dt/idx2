#include "Algorithm.h"
#include "Array.h"
#include "Assert.h"
#include "BitOps.h"
#include "CircularQueue.h"
#include "Common.h"
#include "DataTypes.h"
#include "Function.h"
#include "Logger.h"
#include "Math.h"
#include "Memory.h"
#include "ScopeGuard.h"
#include "nd_wavelet.h"
#include "nd_volume.h"


namespace idx2
{


/*
The TransformTemplate is a string that looks like ':210210:210:210', where each number denotes a dimension.
The transform happens along the dimensions from right to left.
The ':' character denotes level boundary.
The number of ':' characters is the same as the number of levels (which is also Idx2.NLevels).
This is in contrast to v1, in which NLevels is always just 1.
*/
static transform_info_v2
ComputeTransformInfo(stref TransformTemplate, const nd_index& Dims)
{
  transform_info_v2 TransformInfo;
  nd_index CurrentDims = Dims;
  nd_index ExtrapolatedDims = CurrentDims;
  nd_index CurrentSpacing(1); // spacing
  nd_grid G(Dims);
  i8 Pos = TransformTemplate.Size - 1;
  i8 NLevels = 0;
  while (Pos >= 0)
  {
    idx2_Assert(Pos >= 0);
    if (TransformTemplate[Pos--] == ':') // the character ':' (next level)
    {
      G.Spacing = CurrentSpacing;
      G.Dims = CurrentDims;
      ExtrapolatedDims = CurrentDims;
      ++NLevels;
    }
    else // one of 0, 1, 2
    {
      i8 D = TransformTemplate[Pos--] - '0';
      PushBack(&TransformInfo.Grids, G);
      PushBack(&TransformInfo.Axes, D);
      ExtrapolatedDims[D] = CurrentDims[D] + IsEven(CurrentDims[D]);
      G.Dims = ExtrapolatedDims;
      CurrentDims[D] = (ExtrapolatedDims[D] + 1) / 2;
      CurrentSpacing[D] *= 2;
    }
  }
  TransformInfo.BasisNorms = GetCdf53NormsFast<16>();
  TransformInfo.NLevels = NLevels;

  return TransformInfo;
}


/*
Extrapolate a volume to (2^N+1) x (2^N+1) x (2^N+1).
Dims3 are the dimensions of the volume, not Dims(*Vol), which has to be (2^X+1) x (2^Y+1) x (2^Z+1).
*/
static void
ExtrapolateCdf53(const v3i& Dims3, u64 TransformOrder, volume* Vol)
{
  v3i N3 = Dims(*Vol);
  v3i M3(N3.X == 1 ? 1 : N3.X - 1, N3.Y == 1 ? 1 : N3.Y - 1, N3.Z == 1 ? 1 : N3.Z - 1);
  // printf("M3 = " idx2_PrStrV3i "\n", idx2_PrV3i(M3));
  idx2_Assert(IsPow2(M3.X) && IsPow2(M3.Y) && IsPow2(M3.Z));
  int NLevels = Log2Floor(Max(Max(N3.X, N3.Y), N3.Z));
  ForwardCdf53(Dims3, M3, 0, NLevels, TransformOrder, Vol);
  InverseCdf53(N3, M3, 0, NLevels, TransformOrder, Vol);
}


static void
ForwardCdf53(const v3i& M3,
             int Iter,
             const array<subband>& Subbands,
             const transform_info_v2& TransformInfo,
             volume* Vol,
             bool CoarsestLevel)
{
//  idx2_For (int, I, 0, Size(TransformInfo.Axes))
//  {
//    int D = TransformInfo.Axes[I];
//    if (Vol->Type == dtype::float64)
//    {
//      if (D == 0)
//        FLiftCdf53X<f64>(TransformInfo.Grids[I], M3, lift_option::Normal, Vol);
//      else if (D == 1)
//        FLiftCdf53Y<f64>(TransformInfo.Grids[I], M3, lift_option::Normal, Vol);
//      else if (D == 2)
//        FLiftCdf53Z<f64>(TransformInfo.Grids[I], M3, lift_option::Normal, Vol);
//      // TODO: need higher dimensional transforms
//    }
//  }
//
//  /* Optionally normalize */
//  idx2_Assert(IsFloatingPoint(Vol->Type));
//  for (int I = 0; I < Size(Subbands); ++I)
//  {
//    if (I == 0 && !CoarsestLevel)
//      continue; // do not normalize subband 0
//    subband& S = Subbands[I];
//    f64 Wx = M3.X == 1 ? 1
//                       : (S.LowHigh3.X == 0
//                            ? TransformInfo.BasisNorms
//                                .ScalNorms[Iter * TransformInfo.NLevels + S.Level3Rev.X - 1]
//                            : TransformInfo.BasisNorms
//                                .WaveNorms[Iter * TransformInfo.NLevels + S.Level3Rev.X]);
//    f64 Wy = M3.Y == 1 ? 1
//                       : (S.LowHigh3.Y == 0
//                            ? TransformInfo.BasisNorms
//                                .ScalNorms[Iter * TransformInfo.NLevels + S.Level3Rev.Y - 1]
//                            : TransformInfo.BasisNorms
//                                .WaveNorms[Iter * TransformInfo.NLevels + S.Level3Rev.Y]);
//    f64 Idx2 = M3.Z == 1 ? 1
//                         : (S.LowHigh3.Z == 0
//                              ? TransformInfo.BasisNorms
//                                  .ScalNorms[Iter * TransformInfo.NLevels + S.Level3Rev.Z - 1]
//                              : TransformInfo.BasisNorms
//                                  .WaveNorms[Iter * TransformInfo.NLevels + S.Level3Rev.Z]);
//    f64 W = Wx * Wy * Idx2;
//#define Body(type)                                                                                 \
//  auto ItEnd = End<type>(S.Grid, *Vol);                                                            \
//  for (auto It = Begin<type>(S.Grid, *Vol); It != ItEnd; ++It)                                     \
//    *It = type(*It * W);
//    idx2_DispatchOnType(Vol->Type);
//#undef Body
//  }
}


/* The reason we need to know if the input is on the coarsest level is because we do not want
to normalize subband 0 otherwise */
static void
InverseCdf53(const v3i& M3,
             int Iter,
             const array<subband>& Subbands,
             const transform_info& TransformDetails,
             volume* Vol,
             bool CoarsestLevel)
{
  /* inverse normalize if required */
  idx2_Assert(IsFloatingPoint(Vol->Type));
  for (int I = 0; I < Size(Subbands); ++I)
  {
    if (I == 0 && !CoarsestLevel)
      continue; // do not normalize subband 0
    subband& S = Subbands[I];
    f64 Wx = M3.X == 1 ? 1
                       : (S.LowHigh3.X == 0
                            ? TransformDetails.BasisNorms
                                .ScalNorms[Iter * TransformDetails.NPasses + S.Level3Rev.X - 1]
                            : TransformDetails.BasisNorms
                                .WaveNorms[Iter * TransformDetails.NPasses + S.Level3Rev.X]);
    f64 Wy = M3.Y == 1 ? 1
                       : (S.LowHigh3.Y == 0
                            ? TransformDetails.BasisNorms
                                .ScalNorms[Iter * TransformDetails.NPasses + S.Level3Rev.Y - 1]
                            : TransformDetails.BasisNorms
                                .WaveNorms[Iter * TransformDetails.NPasses + S.Level3Rev.Y]);
    f64 Idx2 = M3.Z == 1 ? 1
                         : (S.LowHigh3.Z == 0
                              ? TransformDetails.BasisNorms
                                  .ScalNorms[Iter * TransformDetails.NPasses + S.Level3Rev.Z - 1]
                              : TransformDetails.BasisNorms
                                  .WaveNorms[Iter * TransformDetails.NPasses + S.Level3Rev.Z]);
    f64 W = 1.0 / (Wx * Wy * Idx2);
#define Body(type)                                                                                 \
  auto ItEnd = End<type>(S.Grid, *Vol);                                                            \
  for (auto It = Begin<type>(S.Grid, *Vol); It != ItEnd; ++It)                                     \
    *It = type(*It * W);
    idx2_DispatchOnType(Vol->Type);
#undef Body
  }

  /* perform the inverse transform */
  int I = TransformDetails.StackSize;
  while (I-- > 0)
  {
    int D = TransformDetails.StackAxes[I];
#define Body(type)                                                                                 \
  switch (D)                                                                                       \
  {                                                                                                \
    case 0:                                                                                        \
      ILiftCdf53X<f64>(TransformDetails.StackGrids[I], M3, lift_option::Normal, Vol);              \
      break;                                                                                       \
    case 1:                                                                                        \
      ILiftCdf53Y<f64>(TransformDetails.StackGrids[I], M3, lift_option::Normal, Vol);              \
      break;                                                                                       \
    case 2:                                                                                        \
      ILiftCdf53Z<f64>(TransformDetails.StackGrids[I], M3, lift_option::Normal, Vol);              \
      break;                                                                                       \
    default:                                                                                       \
      idx2_Assert(false);                                                                          \
      break;                                                                                       \
  };
    idx2_DispatchOnType(Vol->Type);
#undef Body
  }
}

static u64
EncodeTransformOrder(const stref& TransformOrder)
{
  u64 Result = 0;
  int Len = Size(TransformOrder);
  for (int I = Len - 1; I >= 0; --I)
  {
    char C = TransformOrder[I];
    idx2_Assert(C == 'X' || C == 'Y' || C == 'Z' || C == '+');
    int T = C == '+' ? 3 : C - 'X';
    Result = (Result << 2) + T;
  }
  return Result;
}



static i8
DecodeTransformOrder(u64 Input, v3i N3, str Output)
{
  N3 = v3i((int)NextPow2(N3.X), (int)NextPow2(N3.Y), (int)NextPow2(N3.Z));
  i8 Len = 0;
  u64 SavedInput = Input;
  while (Prod<u64>(N3) > 1)
  {
    int T = Input & 0x3;
    Input >>= 2;
    if (T == 3)
    {
      if (Input == 3)
        Input = SavedInput;
      else
        SavedInput = Input;
    }
    else
    {
      Output[Len++] = char('X' + T);
      N3[T] >>= 1;
    }
  }
  Output[Len] = '\0';
  return Len;
}


/*
In string form, a TransformOrder is made from 4 characters: X,Y,Z, and +
X, Y, and Z denotes the axis where the transform happens, while + denotes where the next
level begins (any subsequent transform will be done on the coarsest level subband only).
Two consecutive ++ signifies the end of the string. If the end is reached before the number of
levels, the last order gets applied.
In integral form, TransformOrder = T encodes the axis order of the transform in base-4 integers.
T % 4 = D in [0, 3] where 0 = X, 1 = Y, 2 = Z, 3 = +*/
static void
BuildSubbands(const v3i& N3, int NLevels, u64 TransformOrder, array<subband>* Subbands)
{
  Clear(Subbands);
  Reserve(Subbands, ((1 << NumDims(N3)) - 1) * NLevels + 1);
  circular_queue<subband, 256> Queue;
  PushBack(&Queue, subband{ grid(N3), grid(N3), v3<i8>(0), v3<i8>(0), v3<i8>(0), i8(0) });
  i8 Level = 0;
  u64 PrevOrder = TransformOrder;
  v3<i8> MaxLevel3(0);
  stack_array<grid, 32> Grids;
  while (Level < NLevels)
  {
    idx2_Assert(TransformOrder != 0);
    int D = TransformOrder & 0x3;
    TransformOrder >>= 2;
    if (D == 3)
    {                          // next level
      if (TransformOrder == 3) // next one is the last +
        TransformOrder = PrevOrder;
      else
        PrevOrder = TransformOrder;
      i16 Sz = Size(Queue);
      for (i16 I = Sz - 1; I >= 1; --I)
      {
        PushBack(Subbands, Queue[I]);
        PopBack(&Queue);
      }
      ++Level;
      Grids[Level] = Queue[0].Grid;
    }
    else
    {
      ++MaxLevel3[D];
      i16 Sz = Size(Queue);
      for (i16 I = 0; I < Sz; ++I)
      {
        const grid& G = Queue[0].Grid;
        grid_split Gs = SplitAlternate(G, dimension(D));
        v3<i8> NextLevel3 = Queue[0].Level3;
        ++NextLevel3[D];
        v3<i8> NextLowHigh3 = Queue[0].LowHigh3;
        idx2_Assert(NextLowHigh3[D] == 0);
        NextLowHigh3[D] = 1;
        PushBack(&Queue,
                 subband{ Gs.First, Gs.First, NextLevel3, NextLevel3, Queue[0].LowHigh3, Level });
        PushBack(
          &Queue,
          subband{ Gs.Second, Gs.Second, Queue[0].Level3, Queue[0].Level3, NextLowHigh3, Level });
        PopFront(&Queue);
      }
    }
  }
  if (Size(Queue) > 0)
    PushBack(Subbands, Queue[0]);
  Reverse(Begin(Grids), Begin(Grids) + Level + 1);
  i64 Sz = Size(*Subbands);
  for (i64 I = 0; I < Sz; ++I)
  {
    subband& Sband = (*Subbands)[I];
    Sband.Level3 = MaxLevel3 - Sband.Level3Rev;
    Sband.Level = i8(NLevels - Sband.Level - 1);
    Sband.AccumGrid = MergeSubbandGrids(Grids[Sband.Level], Sband.Grid);
  }
  Reverse(Begin(*Subbands), End(*Subbands));
}


grid
MergeSubbandGrids(const grid& Sb1, const grid& Sb2)
{
  v3i Off3 = Abs(From(Sb2) - From(Sb1));
  v3i Strd3 = Min(Strd(Sb1), Strd(Sb2)) * Equals(Off3, v3i(0)) + Off3;
  // TODO: works in case of subbands but not in general
  v3i Dims3 = Dims(Sb1) + NotEquals(From(Sb1), From(Sb2)) * Dims(Sb2);
  v3i From3 = Min(From(Sb2), From(Sb1));
  return grid(From3, Dims3, Strd3);
}


} // namespace idx2