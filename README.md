# meowlang

Embedded math expression language.

## Specification

Just-in-time compiled language will significantly enhance readability and
flexibility of the whole thing.

Currently we use ROOT's TFormula, regular expressions with callbacks and DSuL
in places where some runtime-configured conditions and/or arithmetic
expressions have to be applied.

Yet, at these places a generic solution can be applied, in the manner similar
to DSuL, but with grammar extended and performance leveraged by JIT
compilation.

C+ API usage (pilot namespace is `emdsl` -- for "event math DSL":

    // Create expressions
    emdsl::Expression expr1("SADCHit.eDep + 100");
    emdsl::Expression expr2("SADCHit[kin == HCAL && xIdx == 2 && yIdx == 1].eDep > 1.5 MeV");
    // Evaluate expression on event
    float var = expr1(sadcHit).as<float>();
    // Split entire expression's symbols set onto two parts to set them in
    // different places.
    emdsl::EvalContext ctx21 = expr2.context("kin", "xIdx", "yIdx");
    emdsl::EvalContext ctx22 = expr2.context("SADCHit");
    ctx21 |= did;
    ctx22 |= event;

That in principle would simplify handlers and enhance their performance. For
instance, consider a cut relying on calibration data like:

    - _type: Discriminate
      expr: "sadcHits[ECAL1:1-2--] > 1.5 MeV"
      discriminateEvent: true
    - _type: BiHit_Plot
      exprX: "caloHit[ECAL].eDep"
      exprY: "caloHit[HCAL].eDep"
    - _type: PickMasterTime
      value: "stwtdcHits[STT0:0-32--].time*12.5 - offset"  # offset taken from calib

Variants:
    - Lua JIT
    - YACC+LEX/boost::spirit + clang LLVM/GCC JIT (?)
    - YACC+LEX + callback buffers, like in DSuL
    - third party solution (?)


## Language features

 - Simple logic expression with literals and variable definitions

    1 > 2
    kin == HCAL
    kin == MM && number == 3

 - "nearly equal" expression for floating point numbers comparison
    
    2.34 == (2.33 +/- .01)

 - Math operations

    2 + 3
    3/12
    1e-6^^2.34 / 174.

 - Functions: pow(), sin(), log(), etc.

 - Subset lookup and operations

    a = {1: 23, 2: 34, 3: 45}                       => defines a map
    sadcHits[kin == HCAL && number != 4]            => results a map of SADCHit objects
    caloHits[kin == ECAL].eDep                      => result a map of float values indexed by DetID
    sum( caloHits[kin == ECAL].eDep )               => result a sum over SADCHit objects

 - Vectorized arithmetics
   A certain arithmetics can be applied to the subsets of data to yield
   vectorised result: +, -, /, *, ^ -- in case of `a <op> b`, an operation
   expects the full match. For instance, consider vector sum

    a = {'x': 1, 'y': 2, 'z': 3}
    b = {'x': 3, 'y': 2, 'z': 1}
    a + b

   Vectorised operations by default are done with intersection

    a = {'x': 1, 'y': 2, 'u': 4}
    b = {'x': 3, 'y': 2, 'v': 0}
    a + b       => {'x': 4, 'y': 4}

   To perform arithmetic operation on the intersection:

    a + b

   To perform arithmetic operation on the left/right/both elements preserved:

    a .+ b      => {'x': 4, 'y': 4, 'u': 4}
    a +. b      => {'x': 4, 'y': 4, 'v': 0}
    a .+. b     => {'x': 4, 'y': 4, 'u': 4, 'v': 0}

   To force matching

   a !+ b       => error: `a' is not fully covered by `b'
   a +! b       => error: `b' is not fully covered by `a'
   a !+! b      => 


