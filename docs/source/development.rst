.. include:: global.rst.inc
.. highlight:: bash
.. _development:

Development
===========

This chapter will get you started with |project_name| development.

|project_name| is written in C using Arduino-ESP32.


Contributions
-------------

... are welcome!

Some guidance for contributors:

- discuss about changes on github issue tracker

- make your PRs against the ``master`` branch

- do clean changesets:

  - focus on some topic, resist changing anything else.
  - do not do style changes mixed with functional changes.
  - try to avoid refactorings mixed with functional changes.
  - if you need to fix something after commit/push:

    - if there are ongoing reviews: do a fixup commit you can
      merge into the bad commit later.
    - if there are no ongoing reviews or you did not push the
      bad commit yet: edit the commit to include your fix or
      merge the fixup commit before pushing.
  - have a nice, clear, typo-free commit comment
  - if you fixed an issue, refer to it in your commit comment

- make a pull request on github

- wait for review by other developers


Building a development environment
----------------------------------

TODO


Documentation
-------------

Building the docs with Sphinx
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The documentation (in reStructuredText format, .rst) is in docs/source/.

To build the html version of it, you need to have Sphinx installed.

Now run::

  cd docs/
  make html

Then point a web browser at docs/build/html/index.html.

The website is updated automatically by ReadTheDocs through GitHub web hooks on the
main repository.


Creating binaries
-----------------

Adjust userdefines.h, then use e.g. Arduino IDE -> Sketch -> Export compiled binary.


.. _releasing:

Creating a new release
----------------------

Checklist:

- make sure all issues for this milestone are closed or moved to the
  next milestone
- check if there are any pending fixes for severe issues
- find and fix any low hanging fruit left on the issue tracker
- update ``CHANGES.rst``, based on ``git log $PREVIOUS_RELEASE..``
- check version number of upcoming release in ``CHANGES.rst``
- tag the release::

    git tag -s -m "tagged/signed release Vx.y.z" Vx.y.z

- create binaries

- close release milestone on Github

- create a Github release, include:

  * binaries (see above for how to create them)
  * a link to ``CHANGES.rst``
