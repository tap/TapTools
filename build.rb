#!/usr/bin/ruby -wU
# encoding: utf-8

glibdir = "."
Dir.chdir glibdir
glibdir = Dir.pwd

projectNameParts = glibdir.split('/')
projectName = projectNameParts.last;
projectName.gsub!(/Jamoma/, "")
ENV['JAMOMAPROJECT'] = projectName

@projectName = projectName
@log_root = "./logs-#{@projectName}"
@svn_root = "../#{@projectName}"
@fail_array = Array.new
@zerolink = false

Dir.chdir "#{glibdir}/Core/Foundation"
load "build.rb"
Dir.chdir "#{glibdir}/Core/DSP"
load "build.rb"


Dir.chdir "#{glibdir}/Core/Shared"

require "#{glibdir}/Core/Shared/jamomalib.rb"

create_logs(@projectName)
zero_count
build_project("#{glibdir}/objectivemax/MaxObject", "MaxObject.xcodeproj", "Deployment", true, nil, false)


require "#{glibdir}/max-sdk/source/build.rb"

build_projects_for_dir("#{glibdir}/source")


`rm -rf "#{glibdir}"/TapTools/extensions/tap.loader.*`
`mv "#{glibdir}"/TapTools/externals/tap.loader.* "#{glibdir}"/TapTools/extensions`
`cp "#{glibdir}"/readme.txt "#{glibdir}"/TapTools/readme.txt`


# No support folder because we are using static linking for Jamoma dependencies
#
#`mkdir -p "#{glibdir}"/TapTools/support`
#`cp -r "#{glibdir}"/../../Jamoma/Core/*/library/build/*.dylib "#{glibdir}"/TapTools/support` if mac?
#`rm -f "#{glibdir}"/TapTools/support/*-i386.dylib`
#`rm -f "#{glibdir}"/TapTools/support/*-x86_64.dylib`
#
#`cp -r "#{glibdir}"/../../Jamoma/Core/*/extensions/*/build/*.ttdylib "#{glibdir}"/TapTools/support` if mac?
#`rm -f "#{glibdir}"/TapTools/support/*-i386.ttdylib`
#`rm -f "#{glibdir}"/TapTools/support/*-x86_64.ttdylib`
#
#`cp -r "#{glibdir}"/../../Jamoma/Core/*/library/build/*.dll "#{glibdir}"/TapTools/support` if win?
#
#`cp -r "#{glibdir}"/../../Jamoma/Core/*/extensions/*/build/*.ttdll "#{glibdir}"/TapTools/support` if win?

