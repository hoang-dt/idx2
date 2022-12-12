#pragma once


#include "../idx2Common.h"
#include "../Common.h"
#include "../Array.h"
#include "../Error.h"


namespace idx2
{


struct dimension_info
{
  /* A dimension can either be numerical or categorical. In the */
  array<stref> Names;
  i32 Limit = 0;
  char ShortName = '?';
};

bool
OptVal(i32 NArgs, cstr* Args, cstr Opt, array<dimension_info>* Dimensions);


struct indexing_template
{
  /* A string that looks like
  "fffffttttttttttzzz|zyx:zyx:zyx:zyx:tyx:tyx:tyx:tx:yx:yx:yx:yx:yx:yx"
  The '|' separates the static prefix from the dynamic suffix.
  Only the suffix is used for multiresolution indexing, the prefix is kept constant. */
  stref Full = nullptr;
  /* The zyx:zyx:zyx:zyx:tyx:tyx:tyx:tx:yx:yx:yx:yx:yx:yx part; each : denotes a new level. */
  array<array<i8>> Suffix;
  /* The fffffttttttttttzzz part. */
  array<i8> Prefix;
};

struct idx2_file_v2
{
  static constexpr int MaxNLevels_ = 32;
  static constexpr int MaxNDimsPerLevel_ = 3; // for now our zfp compression only supports up to 3 dimensions

  /* For example, xyztf */
  array<dimension_info> Dimensions;
  indexing_template IdxTemplate;
};

error<idx2_err_code>
Finalize(idx2_file_v2* Idx2);


void
Dealloc(idx2_file_v2* Idx2);


} // namespace idx2

