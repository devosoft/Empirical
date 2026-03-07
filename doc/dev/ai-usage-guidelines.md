# Guidelines for AI-Assisted Development

## Guiding Principle

A human must ultimately take responsibility for all aspects of library code.

## Project Policies

1. **Human Manager Responsibility:** The human manager is responsible for reviewing and fully understanding all contributed code.

2. **Commit Attribution:** Commits may be attributed to an agentic AI tool, but all commit messages should include the `@username` of the human manager who is ultimately responsible for them. For small features, consider squash merges to condense work into a single joint-authored commit to indicate the human associated with any AI contributions.

3. **Pull Request Review Process:** Pull requests involving LLM-assisted code generation or attributed to an agentic AI tool should go through the usual peer review process, with the human manager responsible for the PR completing a pull request code review process first before requesting peer review.

4. **Effort Balance:** The effort contributed by the human manager responsible for LLM-generated code and commits should exceed the effort requested from peer reviewers.

5. **Limits on extent of Agentic AI use:** Agentic AI may be used to perform simple maintenance tasks (e.g., figuring out we need to update a version number) but not to generate new core-library source code.

6. **Energy Safeguards for Agentic AI use:** Agentic AI should not be run automatically and there should be restrictions added to ensure it stops promptly if it is not able to solve a problem.

7. **Research privacy:** Code that has not yet been made public should not be run through an external AI that may add the code to its database.

8. **In-code citations:** Any code that was added via an AI should be clearly marked and ideally include a link to the chat session that generated it.
