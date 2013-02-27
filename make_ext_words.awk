BEGIN {
  print "{ Automatically generated from uforth-ext.h. Do not EDIT!!! }"
  print
}

/^#define/ && NF > 2 {
  printf(": %s ", $5);
  for (i=6; i <= NF; i++) printf("%s ",$i);
  printf("  %d cf ;\n", $3); 
}
