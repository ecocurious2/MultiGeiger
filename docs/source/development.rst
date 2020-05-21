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
  - run the automatic code formatter before committing
  - try to avoid refactorings mixed with functional changes.
  - if you need to fix something after commit/push:

    - if there are ongoing reviews: do a fixup commit you can
      merge into the bad commit later.
    - if there are no ongoing reviews or you did not push the
      bad commit yet: edit the commit to include your fix or
      merge the fixup commit before pushing.
  - have a nice, clear, typo-free commit comment
  - if you fixed an issue, refer to it in your commit comment

- make a pull request on github and check on the PR page
  what ``travis-ci`` tells about the code in your PR

- wait for review by other developers


Building a development environment
----------------------------------

TODO

Automatic Code Formatter
------------------------

We use astyle_ for automated code formatting / formatting checks.

Run it like this:

::

  astyle --options=.astylerc 'multigeiger/\*'

.. _astyle: http://astyle.sourceforge.net/


Documentation
-------------

Building the docs with Sphinx
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The documentation (in reStructuredText format, .rst) is in ``docs/source/``,
``index.rst`` is the starting point there.

To build the docs, you need to have Sphinx_ installed and run:

::

  cd docs/
  make html

Then point a web browser at ``docs/build/html/index.html``.

The website is updated automatically by ReadTheDocs through GitHub web hooks on the
main repository.

After changes of the (english) master docs, the translation master files (``*.pot``)
need updating (adding/removing/updating strings in there):

::

  cd docs/source/
  sphinx-build -b gettext . ../build/gettext

Then, these changes need to get pushed to transifex, so translators can comfortably
translate on the web:

::

  tx push --sources

Later, after translators did their part, updated translations need to get pulled
from transifex:

::

  tx pull --all

Now we have changes in our git workdir and we need to commit them:

::

  git add locales/
  git commit -m "updated translations"
  git push

This will trigger a build of the docs and their translation(s) on readthedocs.io.

.. _Sphinx: https://www.sphinx-doc.org/


Flashing devices / creating binaries
------------------------------------

Arduino IDE:

- do a git checkout of the wanted release, e.g. ``git checkout V1.13.0``
- use the default userdefines.h (available as userdefines-example.h)
- IDE settings:

  - Device: Heltec WiFi Stick  (always use this, even if you have a WiFi Kit 32)
  - Flash size: 4MB (32Mb)
  - Partition scheme: minimal SPIFFS (large APPS with OTA) - this fits onto 4MB devices.
- ``Arduino IDE -> Sketch -> Upload``

  This is to test whether the compiled code actually works after USB-flashing to your device.
- ``Arduino IDE -> Sketch -> Export compiled binary``

  This creates a .bin file for OTA updating. Test whether OTA updating using that file works.


.. _releasing:

Creating a new release
----------------------

Checklist:

- make sure all issues for this milestone are closed or moved to the
  next milestone
- check if there are any pending fixes for severe issues
- check whether some CA certificate (see ``ca_certs.h``) will expire soon and
  whether we already can add their next valid cert.
- find and fix any low hanging fruit left on the issue tracker
- close release milestone on Github
- update ``docs/source/changes.rst``, based on ``git log $PREVIOUS_RELEASE..``
- ``bump2version --new-version 1.23.0 release`` - this will:

  - update versions everywhere
  - auto-create a git tag
  - auto-create a git commit
- review the automatically generated changeset
- create a github release for this tag:

  - create a binary (see above) and attach to the github release
  - add a link to the relevant ``changes.rst`` section to the github release

