package workflows

deny[msg] {
  some i
  input.jobs[i].steps[_].uses
  endswith(lower(input.jobs[i].steps[_].uses), "@main")
  msg := sprintf("Action must be version-pinned, not @main (job: %v)", [i])
}
