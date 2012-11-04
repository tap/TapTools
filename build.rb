#!/usr/bin/env ruby -wKU
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

Dir.chdir "#{glibdir}/../Support"

require "#{glibdir}/../Support/jamomalib.rb"

create_logs(@projectName)
zero_count
build_project("#{glibdir}/../Support/objectivemax/MaxObject", "MaxObject.xcodeproj", "Deployment", true, nil, false)

load "build.rb"

`mkdir -p "#{glibdir}"/max/externals`
`rm -rf "#{glibdir}"/max/externals/*`
`mv "#{glibdir}"/../../Builds/MaxMSP/tap.* "#{glibdir}"/max/externals`
