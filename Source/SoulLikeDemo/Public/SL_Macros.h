#pragma once
#define RETURN_IF_TRUE(Expression) \
    if (Expression) { return; }

#define RETURN_IF_FALSE(Expression) \
    if (!(Expression)) { return; }

#define RETURN_VALUE_IF_TRUE(Expression, ReturnValue) \
    if (Expression) { return ReturnValue; }

#define RETURN_VALUE_IF_FALSE(Expression, ReturnValue) \
    if (!(Expression)) { return ReturnValue; }