A work-in-progress chess bot in C. I previously made a similar chess bot in Java, but I plan to supercede the previous project in terms of functionality and in performance due in part to the increased functionality and optimizations but also because of the more efficient language.

Features implemented as of now:
Bit board representations (ULL's)
Pesto's piece square table eval
Move generation with kindergarten board for rooks (and partially for queens)
alpha beta pruning

Features to be implemented
move ordering
kindergarten board move generation for bishops (and the other half of queen moves)
transposition table
quiescence search
aspiration windows
Lichess API integration
