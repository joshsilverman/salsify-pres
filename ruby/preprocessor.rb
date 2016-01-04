require 'csv'
require 'pry'
require 'nokogiri'
require 'json'
require 'ruby-progressbar'

products_dir = '../data/products'
progressbar = ProgressBar.create
product_count = Dir[File.join(products_dir, '**', '*')].count { |file| File.file?(file) }
# binding.pry

def add_item_to_content(item, contents = "")
  if item.class == String
    contents += "#{item}\n"
  elsif item.class == Hash
    item.each do |key, value|
      if value.class == Hash
        contents = add_item_to_content(value, contents)
      elsif value.class == Array
        value.each do |element|
          contents = add_item_to_content(element, contents)
        end
      else
        contents += "#{key} #{value}\n"
      end
    end
  end

  return contents
end

contents = ""
i = 0
Dir.foreach(products_dir) do |item|
  next if item == '.' or item == '..'

  File.open(products_dir + '/' + item) do |f|
    contents = add_item_to_content(JSON.parse(f.read()), contents)
  end

  i += 1
  if i % (product_count / 100) == 0
    progressbar.increment
  end
end

contents.gsub! '"', ''
contents.gsub! ',', ''
contents.gsub! "\t", ','
contents.gsub! '.', ''
contents = contents.downcase

# parsed = Nokogiri::HTML.parse contents
# contents = parsed.text

open('../data/amazon.txt', 'wb') do |f|
  f.write contents
end
